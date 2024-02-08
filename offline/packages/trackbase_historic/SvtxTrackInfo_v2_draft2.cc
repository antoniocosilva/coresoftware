#include "SvtxTrackInfo_v2_draft2.h"
#include "TrackStateInfo_v1.h"

#include <trackbase/TrkrDefs.h>

#include <cmath>
#include <cstddef>  // for size_t
#include <iostream>
#include <map>
#include <utility>  // for pair

void SvtxTrackInfo_v2_draft2::CopyFrom(const SvtxTrackInfo& source)
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

SvtxTrackInfo_v2_draft2& SvtxTrackInfo_v2_draft2::operator=(const SvtxTrackInfo_v2_draft2& source)
{
  if(this != &source)
  {
    CopyFrom(source);

    set_id(source.get_id());
    set_vertex_id(source.get_vertex_id());

    set_x(STATE::OUTER_TPC, source.get_x(STATE::OUTER_TPC));
    set_y(STATE::OUTER_TPC, source.get_y(STATE::OUTER_TPC));
    set_z(STATE::OUTER_TPC, source.get_z(STATE::OUTER_TPC));
    set_px(STATE::OUTER_TPC, source.get_px(STATE::OUTER_TPC));
    set_py(STATE::OUTER_TPC, source.get_py(STATE::OUTER_TPC));
    set_pz(STATE::OUTER_TPC, source.get_pz(STATE::OUTER_TPC));

    for(int i = 0; i < 21; i++){
      set_covariance(STATE::OUTER_TPC, i, source.get_covariance(STATE::OUTER_TPC, i));
    }
  }
  return *this;
}
