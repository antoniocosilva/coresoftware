/*!
 *  \file		PHActsTrkFitter.C
 *  \brief		Refit SvtxTracks with PHActs.
 *  \details	Refit SvtxTracks with PHActs.
 *  \author		Haiwang Yu <yuhw@nmsu.edu>
 */

#include "PHActsTrkFitter.h"

#include <trackbase/TrkrCluster.h>                  // for TrkrCluster
#include <trackbase/TrkrClusterContainer.h>
#include <trackbase/TrkrDefs.h>
#include <mvtx/MvtxDefs.h>
#include <intt/InttDefs.h>

#include <trackbase_historic/SvtxTrack.h>
#include <trackbase_historic/SvtxTrackMap.h>
#include <trackbase_historic/SvtxTrackMap_v1.h>
#include <trackbase_historic/SvtxTrackState_v1.h>
#include <trackbase_historic/SvtxTrack_v1.h>
#include <trackbase_historic/SvtxVertexMap_v1.h>
#include <trackbase_historic/SvtxVertex_v1.h>
#include <trackbase_historic/SvtxTrackState.h>      // for SvtxTrackState
#include <trackbase_historic/SvtxVertex.h>          // for SvtxVertex
#include <trackbase_historic/SvtxVertexMap.h>       // for SvtxVertexMap

#include <mvtx/MvtxDefs.h>
#include <intt/InttDefs.h>
#include <tpc/TpcDefs.h>

#include <g4detectors/PHG4CylinderGeom.h>           // for PHG4CylinderGeom
#include <g4detectors/PHG4CylinderGeomContainer.h>

//
#include <intt/CylinderGeomIntt.h>

#include <mvtx/CylinderGeom_Mvtx.h>

#include <g4main/PHG4Particle.h>
#include <g4main/PHG4Particlev2.h>
#include <g4main/PHG4TruthInfoContainer.h>
#include <g4main/PHG4VtxPoint.h>                    // for PHG4VtxPoint
#include <g4main/PHG4VtxPointv1.h>

#include <phgeom/PHGeomUtility.h>

#include <fun4all/Fun4AllReturnCodes.h>
#include <fun4all/PHTFileServer.h>
#include <fun4all/SubsysReco.h>                     // for SubsysReco

#include <phool/PHCompositeNode.h>
#include <phool/PHIODataNode.h>
#include <phool/PHNode.h>                           // for PHNode
#include <phool/PHNodeIterator.h>
#include <phool/PHObject.h>                         // for PHObject
#include <phool/getClass.h>
#include <phool/phool.h>

#include <phfield/PHFieldUtility.h>
#include <phgeom/PHGeomUtility.h>

/*
#include "Acts/EventData/Measurement.hpp"
#include "Acts/EventData/MeasurementHelpers.hpp"
#include "Acts/Surfaces/CylinderSurface.hpp"
#include "Acts/Utilities/ParameterDefinitions.hpp"
*/

#include <TRotation.h>
#include <TVector3.h>
#include <TMath.h>                                  // for ATan2
#include <TMatrixT.h>                               // for TMatrixT, operator*
#include <TObject.h>
#include <TGeoManager.h>
#include <TGeoMatrix.h>

#include <cmath>                                   // for sqrt, NAN
#include <iostream>
#include <map>
#include <memory>
#include <utility>
#include <vector>

class PHField;

using namespace std;


/*
 * Constructor
 */
PHActsTrkFitter::PHActsTrkFitter(const string& name)
  : PHTrackFitting(name)
{
  Verbosity(0);

  _event = 0;
}

int PHActsTrkFitter::Setup(PHCompositeNode *topNode)
{
  GetNodes(topNode);

  // create a map of sensor TGeoNode pointers using the TrkrDefs:: hitsetkey as the key  
  MakeTGeoNodeMap(topNode);
  
  return Fun4AllReturnCodes::EVENT_OK;
}

void PHActsTrkFitter::MakeTGeoNodeMap(PHCompositeNode *topNode)
{
  _geomanager = PHGeomUtility::GetTGeoManager(topNode);
  if(!_geomanager )
    {
      cout << PHWHERE << " Did not find TGeoManager, quit! " << endl;
      return;
    }
  TGeoVolume *topVol = _geomanager->GetTopVolume();
  TObjArray *nodeArray = topVol->GetNodes();

  TIter iObj(nodeArray); 
  while(TObject *obj = iObj())
    {
      TGeoNode *node = dynamic_cast<TGeoNode*>(obj);
      std::string node_str = node->GetName();

      std::string mvtx("av_1");
      std::string intt("ladder");
      std::string intt_ext("ladderext");

      if ( node_str.compare(0, mvtx.length(), mvtx) == 0 )       // is it in the MVTX?
	{
	  if(Verbosity() > 100)  cout << " node " << node->GetName() << " is in the MVTX" << endl;
	  getMvtxKeyFromNode(node);
	}
      else if ( node_str.compare(0, intt.length(), intt) ==0 ) 	      // is it in the INTT?
	{
	  // We do not want the "ladderext" nodes
	  if ( node_str.compare(0, intt_ext.length(), intt_ext) ==0 ) 
	    continue;
	  
	  if(Verbosity() > 100) cout << " node " << node->GetName() << " is in the INTT" << endl;	  
	  getInttKeyFromNode(node);
	}
      else
	continue;

      bool print_sensor_paths = false;  // normally false
      if(print_sensor_paths)
	{
	  // Descends the node tree to find the active silicon nodes - used for information only
	  cout<< " Top Node is " << node->GetName() << " volume name is " << node->GetVolume()->GetName()  << endl;
	  cout << " Top Node mother volume name is " << node->GetMotherVolume()->GetName() << endl;
	  isActive(node);
	}
    }
}

void  PHActsTrkFitter::getInttKeyFromNode(TGeoNode *gnode)
{
  int layer = -1;           // sPHENIX layer number
  int itype = -1;           // specifies inner (0) or outer (1) sensor
  int ladder_phi = -1;  // copy number of ladder in phi
  int zposneg = -1;                // specifies positive (1) or negative (0) z
  int ladder_z = -1;      // 0-3, from most negative z to most positive
  
  std::string s = gnode->GetName();
  std::string delimiter = "_";
  std::string posz("posz");
  std::string negz("negz");
  
  size_t pos = 0;
  std::string token;

  int counter = 0;
  while ((pos = s.find(delimiter)) != std::string::npos) {
    token = s.substr(0, pos);
    //std::cout << token << std::endl;
    s.erase(0, pos + delimiter.length());
    if(counter == 1) 
      layer = std::atoi(token.c_str()) + 3;
    if(counter == 2)
      itype = std::atoi(token.c_str());
    if(counter == 3)
      {
	ladder_phi = std::atoi(token.c_str());
	if( s.compare(0, negz.length(), negz) ==0 ) zposneg = 0; 
	if( s.compare(0, posz.length(), posz) ==0 ) zposneg = 1; 
      }	
    counter ++;
  }
  // From SteppingAction:  if(zposneg == 1) ladderz = ladderz_base + 2;  // ladderz = 0, 1 for negative z and = 2, 3 for positive z
  ladder_z = itype  + zposneg*2;  // Check that ladder_base = itype

  // The active sensor is a daughter of gnode
  int ndaught = gnode->GetNdaughters();
  if(ndaught == 0)
    {
      cout << PHWHERE << "OOPS: Did not find INTT sensor! Quit." << endl;
      exit(1);
    }

  std::string intt_refactive("siactive");  
  TGeoNode *sensor_node = 0;
  for(int i=0; i<ndaught; ++i)
    {
      std::string node_str = gnode->GetDaughter(i)->GetName();

      if (node_str.compare(0, intt_refactive.length(), intt_refactive) == 0)
	{
	  sensor_node = gnode->GetDaughter(i);      
	  break;      
	}
    } 
 
  // unique key identifying this sensor
  TrkrDefs::hitsetkey node_key = InttDefs::genHitSetKey(layer, ladder_z, ladder_phi);

  std::pair<TrkrDefs::hitsetkey, TGeoNode*> tmp = make_pair(node_key, sensor_node);
  _cluster_node_map.insert(tmp);

  if(Verbosity() > 1)    
    std::cout << " INTT layer " << layer << " ladder_phi " << ladder_phi << " itype " << itype << " zposneg " << zposneg << " ladder_z " << ladder_z << " name " << sensor_node->GetName() << std::endl;
  
  return;
}

void PHActsTrkFitter::getMvtxKeyFromNode(TGeoNode *gnode)
{
  int counter = 0;
  int impr = -1;   // stave number, 1-48 in TGeo
  int layer = -1;
  int stave = -1;  // derived from impr
  int chip = -1;   // 9 chips per stave

  std::string s = gnode->GetName();
  std::string delimiter = "_";
  
  size_t pos = 0;
  std::string token;

  while ((pos = s.find(delimiter)) != std::string::npos) {
    token = s.substr(0, pos);
    //std::cout << token << std::endl;
    s.erase(0, pos + delimiter.length());
    if(counter == 3) 
      impr = std::atoi(token.c_str());
 
    counter ++;
  }

  // extract layer and stave info from impr
  // int staves_in_layer[3] = {12, 16, 20}; 
  // note - impr stave count starts from 1, not 0, but TrkrCluster counting starts from 0, so we reduce it by 1 here
  impr -= 1;
 
 if(impr < 12)
    {
      layer = 0;
      stave = impr;
    }
  else if(impr > 11 && impr < 28)
    {
      layer = 1;
      stave = impr - 12;
    }
  else
    {
      layer = 2;
      stave = impr - 28;
    }

  // Now descend node tree to find chip ID's - there are multiple chips per stave
  TGeoNode *module_node = gnode->GetDaughter(0);    
  int mnd = module_node->GetNdaughters();
  std::string mvtx_chip("MVTXChip");
  for(int i=0; i<mnd; ++i)
    {
      std::string dstr = module_node->GetDaughter(i)->GetName();
      if (dstr.compare(0, mvtx_chip.length(), mvtx_chip) == 0)
	{
	  if(Verbosity() > 1) 
	    cout << "Found MVTX layer " << layer << " stave " << stave << " chip  " << i << " with node name " <<  module_node->GetDaughter(i)->GetName() << endl;

	  // Make key for this chip
	  TrkrDefs::hitsetkey node_key = MvtxDefs::genHitSetKey(layer, stave, i);

	  // add sensor node to map
	  TGeoNode *sensor_node = module_node->GetDaughter(i)->GetDaughter(0);
	  std::pair<TrkrDefs::hitsetkey, TGeoNode*> tmp = make_pair(node_key, sensor_node);
	  _cluster_node_map.insert(tmp);
	  
	  if(Verbosity() > 1)    
	    std::cout << " MVTX layer " << layer << " stave " << stave << " chip " << chip << " name " << sensor_node->GetName() << std::endl;
	}
    }
  
  return;
}

void PHActsTrkFitter::isActive(TGeoNode *gnode)
{
  // For looking at the node tree only.
  // Recursively searches gnode for silicon sensors, prints out heirarchy

  std::string node_str = gnode->GetName();

  std::string intt_refactive("siactive");
  std::string mvtx_refactive("MVTXSensor");

  if (node_str.compare(0, intt_refactive.length(), intt_refactive) == 0)
    {
      cout << "          ******* Found INTT active volume,  node is " << gnode->GetName() 
	   << " volume name is "   << gnode->GetVolume()->GetName() << endl;

      //const TGeoMatrix* tgMatrix = gnode->GetMatrix();
      //tgMatrix->Print();

      return;
    }
  else if (node_str.compare(0, mvtx_refactive.length(), mvtx_refactive) == 0)
    {
      cout << "          ******* Found MVTX active volume,  node is " << gnode->GetName() 
	   << " volume name is " << gnode->GetVolume()->GetName() << endl;

      //const TGeoMatrix* tgMatrix = gnode->GetMatrix();
      //tgMatrix->Print();

      return;
    }

  int ndaught = gnode->GetNdaughters();
  if(ndaught == 0)
    {
      cout << "     No further daughters" << endl;
    }

  for(int i=0; i<ndaught; ++i)
    {
      cout << "     " << gnode->GetVolume()->GetName() << "  daughter " << i 
	   << " has name " << gnode->GetDaughter(i)->GetVolume()->GetName() << endl;
      isActive(gnode->GetDaughter(i));      
    }
}

int PHActsTrkFitter::Process()
{
  _event++;

  if (Verbosity() > 1)
    {
      std::cout << PHWHERE << "Events processed: " << _event << std::endl;
            cout << "Start PHActsTrkfitter::process_event" << endl;
    }

  // Access the TrkrClusters 
  TrkrClusterContainer::ConstRange clusrange = _clustermap->getClusters();
  for(TrkrClusterContainer::ConstIterator clusiter = clusrange.first; clusiter != clusrange.second; ++clusiter)
    {
      TrkrCluster *cluster = clusiter->second;
      TrkrDefs::cluskey cluskey = clusiter->first;

      // get the cluster parameters in global coordinates
      float x = cluster->getPosition(0);
      float y = cluster->getPosition(1);
      float z = cluster->getPosition(2);

      double radius = sqrt(x*x+y*y);

      // In local coords the covariances are in the  r*phi vs z frame
      // They have been rotated into global coordinates in TrkrCluster

      TMatrixF ERR(3,3);
      for(int i=0; i < 3; ++i)
	for(int j =0; j<3; j++)
	  {
	    ERR[i][j] = cluster->getError(i,j);
	  }

      // extract detector element identifier from cluskey and make Identifier for accessing TGeo element
      unsigned int layer = TrkrDefs::getLayer(cluskey);
      if(Verbosity() > 0) cout << " layer " << layer << endl;

      TVector3 world(x,y,z);
      TVector3 local(0,0,0);
      TMatrixF local_err(3, 3);

      double local_2D[2] = {0};
      double local_err_2D[2][2] = {0};

      unsigned int trkrid = TrkrDefs::getTrkrId(cluskey);  // 0 for MVTX, 1 for INTT, 2 for TPC
      if(trkrid == TrkrDefs::mvtxId)
	{
	  unsigned int staveid = MvtxDefs::getStaveId(cluskey);
	  unsigned int chipid = MvtxDefs::getChipId(cluskey);
	  if(Verbosity() > 0) 
	    cout << "   MVTX cluster with staveid " << staveid << " chipid " << chipid << endl; 

	  // make key for this sensor
	  TrkrDefs::hitsetkey cluster_node_key = MvtxDefs::genHitSetKey(layer, staveid, chipid);
	  // get the TGeoNode for it
	  std::map<TrkrDefs::hitsetkey, TGeoNode*>::iterator it;
	  it = _cluster_node_map.find(cluster_node_key);
	  if(it != _cluster_node_map.end())
	    {
	      TGeoNode *sensor_node = it->second;
	      if(Verbosity() > 0) 
		cout << "       Found in _cluster_node_map: layer " << layer << " staveid " << staveid << " chipid " << chipid 
		     <<  " node " << sensor_node->GetName() << endl;
	    }
	  else
	    {
	      cout << PHWHERE << " Did not find entry in TGeo map for cluster with layer " << layer << " staveid " << staveid 
		   << " chipid " << chipid  << ". That should be impossible!" << endl;
	    }

	  // transform position back to local coords on chip
	  CylinderGeom_Mvtx *layergeom = dynamic_cast<CylinderGeom_Mvtx *>(_geom_container_mvtx->GetLayerGeom(layer));
	  local = layergeom->get_local_from_world_coords(staveid, 0, 0, chipid, world);

	  // rotate errors back to local coords too
	  double ladder_location[3] = {0.0, 0.0, 0.0};
	  // returns the center of the sensor in world coordinates - used to get the ladder phi location
	  layergeom->find_sensor_center(staveid, 0, 0, chipid, ladder_location);
	  double ladderphi = atan2(ladder_location[1], ladder_location[0]);

	  TMatrixF ROT(3, 3);
	  ROT[0][0] = cos(ladderphi);
	  ROT[0][1] = -1.0 * sin(ladderphi);
	  ROT[0][2] = 0.0;
	  ROT[1][0] = sin(ladderphi);
	  ROT[1][1] = cos(ladderphi);
	  ROT[1][2] = 0.0;
	  ROT[2][0] = 0.0; 
	  ROT[2][1] = 0.0;
	  ROT[2][2] = 1.0;

	  ROT.Invert();

	  TMatrixF ROT_T(3, 3);
	  ROT_T.Transpose(ROT);
	  
	  local_err = ROT * ERR * ROT_T;

	  if(Verbosity() > 1)
	    {
	      for(int i=0;i<3;++i)
		{
		  cout << " i " << i << " local 3D " << local[i] << endl;
		}
	      for(int i=0;i<3;++i)
		for(int j = 0; j<3; ++j)
		  {
		    cout << "  " << i << "    " << j << " local_err 3D " << local_err[i][j] << endl;
		  }
	    }
	  
	  local_2D[0] = local[0];
	  local_2D[1] = local[2];
	  local_err_2D[0][0] = local_err[1][1];
	  local_err_2D[0][1] = local_err[1][2];
	  local_err_2D[1][0] = local_err[2][1];
	  local_err_2D[1][1] = local_err[2][2];

	}
      else if (trkrid == TrkrDefs::inttId)
	{
	  unsigned int ladderzid = InttDefs::getLadderZId(cluskey);
	  unsigned int ladderphiid = InttDefs::getLadderPhiId(cluskey);
	  if(Verbosity() > 0) 
	    cout << "   Intt cluster with ladderzid " << ladderzid << " ladderphid " << ladderphiid << endl; 

	  // make identifier for this sensor
	  TrkrDefs::hitsetkey cluster_node_key = InttDefs::genHitSetKey(layer, ladderzid, ladderphiid);
	  // get the TGeoNode for it
	  std::map<TrkrDefs::hitsetkey, TGeoNode*>::iterator it;
	  it = _cluster_node_map.find(cluster_node_key);
	  if(it != _cluster_node_map.end())
	    {
	      TGeoNode *sensor_node = it->second;
	      if(Verbosity() > 0)
		cout << "      Found in _cluster_node_map:  layer " << layer << " ladderzid " << ladderzid << " ladderphiid " << ladderphiid 
		     <<  " node " << sensor_node->GetName() << endl;;
	    }
	  else
	    cout << PHWHERE << " Did not find entry in TGeo map for this cluster. That should be impossible!" << endl;

	  // transform position back to local coords on sensor
	  // TBD! convert world to local for INTT
	  CylinderGeomIntt *layergeom = dynamic_cast<CylinderGeomIntt *>(_geom_container_intt->GetLayerGeom(layer));

	  //local = layergeom->get_local_from_world_coords(staveid, 0, 0, chipid, world);

	  // rotate errors back to local coords too	
	  double ladder_location[3] = {0.0, 0.0, 0.0};
	  layergeom->find_segment_center(ladderzid,
				    ladderphiid,
				    ladder_location);
	  double ladderphi = atan2(ladder_location[1], ladder_location[0]);
	  
	  TMatrixF ROT(3, 3);
	  ROT[0][0] = cos(ladderphi);
	  ROT[0][1] = -1.0 * sin(ladderphi);
	  ROT[0][2] = 0.0;
	  ROT[1][0] = sin(ladderphi);
	  ROT[1][1] = cos(ladderphi);
	  ROT[1][2] = 0.0;
	  ROT[2][0] = 0.0; 
	  ROT[2][1] = 0.0;
	  ROT[2][2] = 1.0;

	  ROT.Invert();

	  TMatrixF ROT_T(3, 3);
	  ROT_T.Transpose(ROT);
	  
	  TMatrixF local_err(3, 3);
	  local_err = ROT * ERR * ROT_T;

	  if(Verbosity() > 1)
	    {
	      for(int i=0;i<3;++i)
		{
		  cout << " i " << i << " local 3D " << local[i] << endl;
		}
	      for(int i=0;i<3;++i)
		for(int j = 0; j<3; ++j)
		  {
		    cout << "    " << i << "   " << j << " local_err 3D " << local_err[i][j] << endl;
		  }
	    }

	  local_2D[0] = local[1];
	  local_2D[1] = local[2];
	  local_err_2D[0][0] = local_err[1][1];
	  local_err_2D[0][1] = local_err[1][2];
	  local_err_2D[1][0] = local_err[2][1];
	  local_err_2D[1][1] = local_err[2][2];
	}
      else  // TPC
	{
	  /*
	  unsigned int sectorid = TpcDefs::getSectorId(cluskey);
	  unsigned int side = TpcDefs::getSide(cluskey);
	  unsigned int pad = TpcDefs::getPad(cluskey);
	  unsigned int tbin = TpcDefs::getTBin(cluskey);
	  */

	  // transform position local coords on cylinder, at center of layer
	  // What do we mean by local coords on a cylinder?
	  // has to be phi and z, right?
	  // so it is just the phi and z part of the global coords

	  double clusphi = atan2(world[1], world[0]);
	  double r_clusphi = radius*clusphi;
	  double ztpc = world[2];

	  // rotate errors back to local coords too	
	  TMatrixF ROT(3, 3);
	  ROT[0][0] = cos(clusphi);
	  ROT[0][1] = -1.0 * sin(clusphi);
	  ROT[0][2] = 0.0;
	  ROT[1][0] = sin(clusphi);
	  ROT[1][1] = cos(clusphi);
	  ROT[1][2] = 0.0;
	  ROT[2][0] = 0.0; 
	  ROT[2][1] = 0.0;
	  ROT[2][2] = 1.0;

	  ROT.Invert();

	  TMatrixF ROT_T(3, 3);
	  ROT_T.Transpose(ROT);
	  
	  TMatrixF local_err(3, 3);
	  local_err = ROT * ERR * ROT_T;

	  if(Verbosity() > 1)
	    {
	      cout << " r " <<  " local 3D " << radius << endl;
	      cout << " r-phi " <<  " local 3D " << r_clusphi << endl;
	      cout << " z " << " local 3D " << ztpc << endl;
	      
	      for(int i=0;i<3;++i)
		for(int j = 0; j<3; ++j)
		  {
		    cout << "   " << i << "   " << j << " local_err 3D " << local_err[i][j] << endl;
		  }
	    }
      
	  local_2D[0] = r_clusphi;
	  local_2D[1] = ztpc;
	  local_err_2D[0][0] = local_err[1][1];
	  local_err_2D[0][1] = local_err[1][2];
	  local_err_2D[1][0] = local_err[2][1];
	  local_err_2D[1][1] = local_err[2][2];
	}

      // local and local_err now contain the position and covariance matrix in local coords
      if(Verbosity() > 1)
	{
	  for(int i=0;i<2;++i)
	    {
	      cout << " i " << i << " local_2D " << local_2D[i]  << endl;
	    }
	  for(int i=0;i<2;++i)
	    for(int j=0;j<2;++j)
	      {
		cout << "   " << i << "   " << j << " cov_2D " << local_err_2D[i][j] << endl;	      	      
	      }
	}

    }

  /*
  // create Acts measurement container
  
  */


  // _trackmap is SvtxTrackMap from the node tree
  // We need to convert to Acts tracks
  for (SvtxTrackMap::Iter iter = _trackmap->begin(); iter != _trackmap->end();
       ++iter)
  {
    SvtxTrack* svtx_track = iter->second;
    if(Verbosity() > 0)
      {
	cout << "   found SVTXTrack " << iter->first << endl;
	svtx_track->identify();
      }
    if (!svtx_track)
      continue;

  }
  return 0;
}

/*
 * End
 */
int PHActsTrkFitter::End(PHCompositeNode* topNode)
{

  return Fun4AllReturnCodes::EVENT_OK;
}

/*
 * dtor
 */
PHActsTrkFitter::~PHActsTrkFitter()
{

}


int PHActsTrkFitter::CreateNodes(PHCompositeNode* topNode)
{

  return Fun4AllReturnCodes::EVENT_OK;
}

/*
 * GetNodes():
 *  Get all the all the required nodes off the node tree
 */
int PHActsTrkFitter::GetNodes(PHCompositeNode* topNode)
{
  _geom_container_mvtx = findNode::getClass<
    PHG4CylinderGeomContainer>(topNode, "CYLINDERGEOM_MVTX");
  if (!_geom_container_mvtx)
  {
    cout << PHWHERE << " CYLINDERGEOM_MVTX  node not found on node tree"
         << endl;
    return Fun4AllReturnCodes::ABORTEVENT;
  }

  _geom_container_intt = findNode::getClass<
    PHG4CylinderGeomContainer>(topNode, "CYLINDERGEOM_INTT");
  if (!_geom_container_intt)
    {
    cout << PHWHERE << " CYLINDERGEOM_INTT  node not found on node tree"
         << endl;
    return Fun4AllReturnCodes::ABORTEVENT;
  }

  // Input Trkr Clusters
  _clustermap = findNode::getClass<TrkrClusterContainer>(topNode, "TRKR_CLUSTER");
  if (!_clustermap)
  {
    cout << PHWHERE << " TRKR_CLUSTER node not found on node tree"
         << endl;
    return Fun4AllReturnCodes::ABORTEVENT;
  }

  // Input Svtx Tracks
  _trackmap = findNode::getClass<SvtxTrackMap>(topNode, "SvtxTrackMap");
  if (!_trackmap)
  {
    cout << PHWHERE << " SvtxTrackMap node not found on node tree"
         << endl;
    return Fun4AllReturnCodes::ABORTEVENT;
  }

  /*
  // Input Svtx Vertices
  _vertexmap = findNode::getClass<SvtxVertexMap>(topNode, "SvtxVertexMap");
  if (!_vertexmap && _event < 2)
  {
    cout << PHWHERE << " SvtxVertexrMap node not found on node tree"
         << endl;
    return Fun4AllReturnCodes::ABORTEVENT;
  }
  */

  /*(
  // Output Svtx Tracks
  if (!(_over_write_svtxtrackmap) || _output_mode == DebugMode)
  {
    _trackmap_refit = findNode::getClass<SvtxTrackMap>(topNode,
                                                       "SvtxTrackMapRefit");
    if (!_trackmap_refit && _event < 2)
    {
      cout << PHWHERE << " SvtxTrackMapRefit node not found on node tree"
           << endl;
      return Fun4AllReturnCodes::ABORTEVENT;
    }
  }
  */

  return Fun4AllReturnCodes::EVENT_OK;
}



