#ifndef __CAHNHILLIARD_THERMAL_H__
#define __CAHNHILLIARD_THERMAL_H__

#include <vector>
#include <algorithm>
#include "chparams.h"

class CahnHilliard2DRHS_thermal {

 public:

  CahnHilliard2DRHS_thermal(CHparamsScalar& chp , SimInfo& info);
  CahnHilliard2DRHS_thermal(CHparamsVector& chp , SimInfo& info);
  ~CahnHilliard2DRHS_thermal();
  void rhs(const std::vector<double> &c, std::vector<double> &dcdt, const double t);
  void operator()(const std::vector<double> &c, std::vector<double> &dcdt, const double t);
  void setInitialConditions(std::vector<double> &x);
  double l2residual(const std::vector<double> &c);
  CHparamsVector compute_chparams_using_temperature( CHparamsVector& chpV0 , std::vector<double> T );
  void write_state( const std::vector<double> &x , const int idx , const int nx );
  
 private:

  CHparamsVector chpV_;
  SimInfo& info_;

  std::default_random_engine generator_;
  std::normal_distribution<double> noise_dist_;
  
  double laplace_component(int i ,
			   const std::vector<double>& c ,
			   const std::vector<double>& u ,
			   const std::vector<double>& b);
  int idx2d_impl(int i, int j);
  int mod(int a, int b);
  int idx2d(int i, int j);
  
};





#endif