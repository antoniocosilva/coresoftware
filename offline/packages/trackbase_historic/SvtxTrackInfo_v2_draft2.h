#ifndef TRACKBASEHISTORIC_SVTXTRACKINFOV2_DRAFT2_H
#define TRACKBASEHISTORIC_SVTXTRACKINFOV2_DRAFT2_H

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

class SvtxTrackInfo_v2_draft2: public SvtxTrackInfo
{
 public:
  enum STATE
  {
    VERTEX = 0,
    OUTER_TPC = 1
  };

  SvtxTrackInfo_v2_draft2() {}

  //* base class copy constructor
  SvtxTrackInfo_v2_draft2( const SvtxTrackInfo& ) {}

  //* copy constructor
  SvtxTrackInfo_v2_draft2(const SvtxTrackInfo_v2_draft2& source){
    m_track_id = source.get_id();
    m_chisq = source.get_chisq();
    m_ndf = source.get_ndf();
    m_crossing = source.get_crossing();
    m_hitbitmap = source.get_hitbitmap();

    m_state.push_back(TrackStateInfo_v1());
    m_state.push_back(TrackStateInfo_v1());

    set_x(source.get_x());
    set_y(source.get_y());
    set_z(source.get_z());
    set_px(source.get_px());
    set_py(source.get_py());
    set_pz(source.get_pz());

    set_x(STATE::OUTER_TPC, source.get_x(STATE::OUTER_TPC));
    set_y(STATE::OUTER_TPC, source.get_y(STATE::OUTER_TPC));
    set_z(STATE::OUTER_TPC, source.get_z(STATE::OUTER_TPC));
    set_px(STATE::OUTER_TPC, source.get_px(STATE::OUTER_TPC));
    set_py(STATE::OUTER_TPC, source.get_py(STATE::OUTER_TPC));
    set_pz(STATE::OUTER_TPC, source.get_pz(STATE::OUTER_TPC));

    for(int i = 0; i < 21; i++){
      set_covariance(i, source.get_covariance(i));
      set_covariance(STATE::OUTER_TPC, i, source.get_covariance(STATE::OUTER_TPC, i));
    }

  }

  //* assignment operator
  SvtxTrackInfo_v2_draft2& operator=(const SvtxTrackInfo_v2_draft2& track);

  //* destructor
  ~SvtxTrackInfo_v2_draft2() override {}

  // The "standard PHObject response" functions...
  void identify(std::ostream& os = std::cout) const override {
    os << "SvtxTrackInfo_v2_draft2 class" << std::endl;
  }
  void Reset() override { *this = SvtxTrackInfo_v2_draft2(); }
  //int isValid() const override;
  PHObject* CloneMe() const override { return new SvtxTrackInfo_v2_draft2(*this); }

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

  float get_x() const override { return m_state.at(STATE::VERTEX).get_x(); }
  void set_x(float x) override { m_state.at(STATE::VERTEX).set_x(x); }

  float get_y() const override { return m_state.at(STATE::VERTEX).get_y(); }
  void set_y(float y) override { m_state.at(STATE::VERTEX).set_y(y); }

  float get_z() const override { return m_state.at(STATE::VERTEX).get_z(); }
  void set_z(float z) override { m_state.at(STATE::VERTEX).set_z(z); }

  float get_pos(unsigned int i) const override { return m_state.at(STATE::VERTEX).get_pos(i); }

  float get_px() const override { return m_state.at(STATE::VERTEX).get_px(); }
  void set_px(float px) override { m_state.at(STATE::VERTEX).set_px(px); }

  float get_py() const override { return m_state.at(STATE::VERTEX).get_py(); }
  void set_py(float py) override { m_state.at(STATE::VERTEX).set_py(py); }

  float get_pz() const override { return m_state.at(STATE::VERTEX).get_pz(); }
  void set_pz(float pz) override { m_state.at(STATE::VERTEX).set_pz(pz); }

  float get_mom(unsigned int i) const override { return m_state.at(STATE::VERTEX).get_mom(i); }

  float get_p() const override { return sqrt(pow(get_px(), 2) + pow(get_py(), 2) + pow(get_pz(), 2)); }
  float get_pt() const override { return sqrt(pow(get_px(), 2) + pow(get_py(), 2)); }
  float get_eta() const override { return asinh(get_pz() / get_pt()); }
  float get_phi() const override { return atan2(get_py(), get_px()); }

  float get_covariance(int i) const override { return m_state.at(STATE::VERTEX).get_covariance(i);}
  void set_covariance(int i, float value) override {m_state.at(STATE::VERTEX).set_covariance(i, value);}

  float get_x(STATE state) const { return m_state.at(state).get_x(); }
  void set_x(STATE state, float x) { m_state.at(state).set_x(x); }

  float get_y(STATE state) const { return m_state.at(state).get_y(); }
  void set_y(STATE state, float y) { m_state.at(state).set_y(y); }

  float get_z(STATE state) const { return m_state.at(state).get_z(); }
  void set_z(STATE state, float z) { m_state.at(state).set_z(z); }

  float get_pos(STATE state, unsigned int i) const { return m_state.at(state).get_pos(i); }

  float get_px(STATE state) const { return m_state.at(state).get_px(); }
  void set_px(STATE state, float px) { m_state.at(state).set_px(px); }

  float get_py(STATE state) const { return m_state.at(state).get_py(); }
  void set_py(STATE state, float py) { m_state.at(state).set_py(py); }

  float get_pz(STATE state) const { return m_state.at(state).get_pz(); }
  void set_pz(STATE state, float pz) { m_state.at(state).set_pz(pz); }

  float get_mom(STATE state, unsigned int i) const { return m_state.at(state).get_mom(i); }

  float get_p(STATE state) const { return sqrt(pow(get_px(state), 2) + pow(get_py(state), 2) + pow(get_pz(state), 2)); }
  float get_pt(STATE state) const { return sqrt(pow(get_px(state), 2) + pow(get_py(state), 2)); }
  float get_eta(STATE state) const { return asinh(get_z(state) / sqrt(get_x(state)*get_x(state) + get_y(state)+get_y(state))); }
  float get_phi(STATE state) const { return atan2(get_y(state), get_x(state)); }

  //float get_error(int i, int j) const override { return _states.find(0.0)->second->get_error(i, j); }
  //void set_error(int i, int j, float value) override { return _states[0.0]->set_error(i, j, value); }

  float get_covariance(STATE state, int i) const { return m_state.at(state).get_covariance(i);}
  void set_covariance(STATE state, int i, float value) {m_state.at(state).set_covariance(i, value);}

 private:

  // track information
  unsigned int m_track_id = UINT_MAX;
  unsigned int m_vertex_id = UINT_MAX;
  float m_chisq = NAN;
  uint8_t m_ndf = uint8_t(-1);
  uint64_t m_hitbitmap = uint64_t(-1);
  short int m_crossing = SHRT_MAX;

  // track state information
  std::vector<TrackStateInfo_v1> m_state;

  ClassDefOverride(SvtxTrackInfo_v2_draft2, 1)
};

#endif
