#ifndef TRACKBASEHISTORIC_SVTXTRACKINFOV2_DRAFT1_H
#define TRACKBASEHISTORIC_SVTXTRACKINFOV2_DRAFT1_H

#include "SvtxTrackInfo.h"
#include "TrackStateInfo_v1.h"

#include <trackbase/TrkrDefs.h>

#include <cmath>
#include <cstddef>  // for size_t
#include <iostream>
#include <map>
#include <utility>  // for pair
#include <cstdint>

class PHObject;

class SvtxTrackInfo_v2_draft1: public SvtxTrackInfo
{
 public:
  SvtxTrackInfo_v2_draft1() {}

  //* base class copy constructor
  SvtxTrackInfo_v2_draft1( const SvtxTrackInfo& ) {}

  //* copy constructor
  SvtxTrackInfo_v2_draft1(const SvtxTrackInfo_v2_draft1& source){
    m_track_id = source.get_id();
    m_chisq = source.get_chisq();
    m_ndf = source.get_ndf();
    m_crossing = source.get_crossing();
    m_hitbitmap = source.get_hitbitmap();

    set_x(source.get_x());
    set_y(source.get_y());
    set_z(source.get_z());
    set_px(source.get_px());
    set_py(source.get_py());
    set_pz(source.get_pz());

    set_x_outer_tpc(source.get_x_outer_tpc());
    set_y_outer_tpc(source.get_y_outer_tpc());
    set_z_outer_tpc(source.get_z_outer_tpc());
    set_px_outer_tpc(source.get_px_outer_tpc());
    set_py_outer_tpc(source.get_py_outer_tpc());
    set_pz_outer_tpc(source.get_pz_outer_tpc());

    for(int i = 0; i < 21; i++){
      set_covariance(i, source.get_covariance(i));
      set_covariance_outer_tpc(i, source.get_covariance_outer_tpc(i));
    }

  }

  //* assignment operator
  SvtxTrackInfo_v2_draft1& operator=(const SvtxTrackInfo_v2_draft1& track);

  //* destructor
  ~SvtxTrackInfo_v2_draft1() override {}

  // The "standard PHObject response" functions...
  void identify(std::ostream& os = std::cout) const override {
    os << "SvtxTrackInfo_v2_draft1 class" << std::endl;
  }
  void Reset() override { *this = SvtxTrackInfo_v2_draft1(); }
  //int isValid() const override;
  PHObject* CloneMe() const override { return new SvtxTrackInfo_v2_draft1(*this); }

  //! import PHObject CopyFrom, in order to avoid clang warning
  using PHObject::CopyFrom;
  // copy content from base class
  void CopyFrom( const SvtxTrackInfo& ) override;
  void CopyFrom( SvtxTrackInfo* source ) override
  { CopyFrom( *source ); }

  //
  // basic track information ---------------------------------------------------
  //

  unsigned int get_id() const { return m_track_id; }
  void set_id(unsigned int id) { m_track_id = id; }

  unsigned int get_vertex_id() const { return m_vertex_id; }
  void set_vertex_id(unsigned int id) { m_vertex_id = id; }

  float get_chisq() const override { return m_chisq; }
  void set_chisq(float chisq) override { m_chisq = chisq; }

  uint8_t get_ndf() const override { return m_ndf; }
  void set_ndf(uint8_t ndf) override { m_ndf = ndf; }

  uint64_t get_hitbitmap() const override { return m_hitbitmap; }
  void set_hitbitmap(uint64_t hitbitmap) override { m_hitbitmap = hitbitmap; }

  short int get_crossing() const override { return m_crossing; }
  void set_crossing(short int crossing) override { m_crossing = crossing; }

  float get_x() const override { return m_state_vertex.get_x(); }
  void set_x(float x) override { m_state_vertex.set_x(x); }

  float get_y() const override { return m_state_vertex.get_y(); }
  void set_y(float y) override { m_state_vertex.set_y(y); }

  float get_z() const override { return m_state_vertex.get_z(); }
  void set_z(float z) override { m_state_vertex.set_z(z); }

  float get_pos(unsigned int i) const override { return m_state_vertex.get_pos(i); }

  float get_px() const override { return m_state_vertex.get_px(); }
  void set_px(float px) override { m_state_vertex.set_px(px); }

  float get_py() const override { return m_state_vertex.get_py(); }
  void set_py(float py) override { m_state_vertex.set_py(py); }

  float get_pz() const override { return m_state_vertex.get_pz(); }
  void set_pz(float pz) override { m_state_vertex.set_pz(pz); }

  float get_mom(unsigned int i) const override { return m_state_vertex.get_mom(i); }

  float get_p() const override { return sqrt(pow(get_px(), 2) + pow(get_py(), 2) + pow(get_pz(), 2)); }
  float get_pt() const override { return sqrt(pow(get_px(), 2) + pow(get_py(), 2)); }
  float get_eta() const override { return asinh(get_pz() / get_pt()); }
  float get_phi() const override { return atan2(get_py(), get_px()); }

  float get_covariance(int i) const override { return m_state_vertex.get_covariance(i);}
  void set_covariance(int i, float value) override {m_state_vertex.set_covariance(i, value);}

  float get_x_outer_tpc() const { return m_state_outer_tpc.get_x(); }
  void set_x_outer_tpc(float x) { m_state_outer_tpc.set_x(x); }

  float get_y_outer_tpc() const { return m_state_outer_tpc.get_y(); }
  void set_y_outer_tpc(float y) { m_state_outer_tpc.set_y(y); }

  float get_z_outer_tpc() const { return m_state_outer_tpc.get_z(); }
  void set_z_outer_tpc(float z) { m_state_outer_tpc.set_z(z); }

  float get_pos_outer_tpc(unsigned int i) const { return m_state_outer_tpc.get_pos(i); }

  float get_px_outer_tpc() const { return m_state_outer_tpc.get_px(); }
  void set_px_outer_tpc(float px) { m_state_outer_tpc.set_px(px); }

  float get_py_outer_tpc() const { return m_state_outer_tpc.get_py(); }
  void set_py_outer_tpc(float py) { m_state_outer_tpc.set_py(py); }

  float get_pz_outer_tpc() const { return m_state_outer_tpc.get_pz(); }
  void set_pz_outer_tpc(float pz) { m_state_outer_tpc.set_pz(pz); }

  float get_mom_outer_tpc(unsigned int i) const { return m_state_outer_tpc.get_mom(i); }

  float get_p_outer_tpc() const { return sqrt(pow(get_px_outer_tpc(), 2) + pow(get_py_outer_tpc(), 2) + pow(get_pz_outer_tpc(), 2)); }
  float get_pt_outer_tpc() const { return sqrt(pow(get_px_outer_tpc(), 2) + pow(get_py_outer_tpc(), 2)); }
  float get_eta_outer_tpc() const { return asinh(get_z_outer_tpc() / sqrt(get_x_outer_tpc()*get_x_outer_tpc() + get_y_outer_tpc()*get_y_outer_tpc())); }
  float get_phi_outer_tpc() const { return atan2(get_y_outer_tpc(), get_x_outer_tpc()); }

  float get_covariance_outer_tpc(int i) const { return m_state_outer_tpc.get_covariance(i);}
  void set_covariance_outer_tpc(int i, float value) {m_state_outer_tpc.set_covariance(i, value);}

 private:

  // track information
  unsigned int m_track_id = UINT_MAX;
  unsigned int m_vertex_id = UINT_MAX;
  float m_chisq = NAN;
  uint8_t m_ndf = uint8_t(-1);
  uint64_t m_hitbitmap = uint64_t(-1);
  short int m_crossing = SHRT_MAX;

  // track state information
  TrackStateInfo_v1 m_state_vertex;
  TrackStateInfo_v1 m_state_outer_tpc;

  ClassDefOverride(SvtxTrackInfo_v2_draft1, 1)
};

#endif
