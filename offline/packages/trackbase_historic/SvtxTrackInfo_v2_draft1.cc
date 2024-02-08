#include "SvtxTrackInfo_v2_draft1.h"
#include "TrackStateInfo_v1.h"

#include <trackbase/TrkrDefs.h>

#include <cmath>
#include <cstddef>  // for size_t
#include <iostream>
#include <map>
#include <utility>  // for pair

void SvtxTrackInfo_v2_draft1::CopyFrom(const SvtxTrackInfo& source)
{
  set_chisq(source.get_chisq());
  set_ndf(source.get_ndf());
  set_hitbitmap(source.get_hitbitmap());
  set_crossing(source.get_crossing());

  set_x(source.get_x());
  set_y(source.get_y());
  set_z(source.get_z());
  set_px(source.get_px());
  set_py(source.get_py());
  set_pz(source.get_pz());

  for(int i = 0; i < 21; i++){
    set_covariance(i, source.get_covariance(i));
  }
}

SvtxTrackInfo_v2_draft1& SvtxTrackInfo_v2_draft1::operator=(const SvtxTrackInfo_v2_draft1& source)
{
  if(this != &source)
  {
    CopyFrom(source);

    set_id(source.get_id());
    set_vertex_id(source.get_vertex_id());

    set_x_outer_tpc(source.get_x_outer_tpc());
    set_y_outer_tpc(source.get_y_outer_tpc());
    set_z_outer_tpc(source.get_z_outer_tpc());
    set_px_outer_tpc(source.get_px_outer_tpc());
    set_py_outer_tpc(source.get_py_outer_tpc());
    set_pz_outer_tpc(source.get_pz_outer_tpc());

    for(int i = 0; i < 21; i++){
      set_covariance_outer_tpc(i, source.get_covariance_outer_tpc(i));
    }
  }
  return *this;
}
