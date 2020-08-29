//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KernelDensity1D.h"
#include "math.h"
#include "libmesh/utility.h"
#include "DelimitedFileReader.h"
#include "SystemBase.h"
#include "Assembly.h"
#include "Normal.h"
#include "Uniform.h"

registerMooseObject("StochasticToolsApp", KernelDensity1D);

InputParameters
KernelDensity1D::validParams()
{
  InputParameters params = Distribution::validParams();
  MooseEnum bandwidthrule("silverman standarddeviation userdefined");
  MooseEnum dataformat("vector filename");
  MooseEnum kernelfunction("gaussian uniform");
  params.addClassDescription("KernelDensity1D distribution");
  params.addRequiredParam<MooseEnum>("bandwidthrule", bandwidthrule, "Bandwidth rule for evaluating the bandwith.");
  params.addRequiredParam<MooseEnum>("dataformat", dataformat, "Either a data vector or a CSV filename should be provided for building the kernel density.");
  params.addRequiredParam<MooseEnum>("kernelfunction", kernelfunction, "Kernel function determines the shape of the underlying kernel for the kernel density.");
  params.addRangeCheckedParam<Real>(
      "bandwidth", 1.0, "bandwidth > 0", "Bandwidth controls the smoothness of the kernel density.");
  params.addParam<std::vector<Real>>("data","The data vector.");
  params.addParam<FileName>("file_name", "Name of the CSV file.");
  return params;
}

KernelDensity1D::KernelDensity1D(const InputParameters & parameters)
  : Distribution(parameters),
    _bandwidthrule(getParam<MooseEnum>("bandwidthrule")),
    _dataformat(getParam<MooseEnum>("dataformat")),
    _kernelfunction(getParam<MooseEnum>("kernelfunction")),
    _bandwidth(getParam<Real>("bandwidth")),
    _data_set(parameters.isParamSetByUser("data")),
    _file_name_set(parameters.isParamSetByUser("file_name")),
    _data(getParam<std::vector<Real>>("data")),
    _file_name(getParam<FileName>("file_name"))
{
  if (_dataformat == "vector" && !_data_set){
    paramError("data",
               "A data vector needs to be provided if data format equals vector is chosen.");
  }
  if (_dataformat == "filename" && !_file_name_set){
    paramError("file_name",
               "A CSV file name needs to be provided if data format equals file_name is chosen.");
  }
  if (_dataformat == "filename"){
    MooseUtils::DelimitedFileReader reader(_file_name);
    reader.setHeaderFlag(MooseUtils::DelimitedFileReader::HeaderFlag::OFF);
    reader.read();
    std::vector<std::vector<Real>> data1 = reader.getData();
    _data = data1[0];
  }
  Real mu = 0;
  Real sd = 0;
  for (unsigned i = 0; i < _data.size(); ++i){
    mu += _data[i] /  _data.size();
  }
  for (unsigned i = 0; i < _data.size(); ++i){
    sd += std::pow((_data[i] - mu),2) /  _data.size();
  }
  sd = std::pow(sd, 0.5);
  if (_bandwidthrule == "silverman"){
    _bandwidth = 1.06 * sd * std::pow(_data.size(), -0.2);
  } else if (_bandwidthrule == "standarddeviation"){
    _bandwidth = sd;
  }
}

Real
KernelDensity1D::pdf(const Real & x, const Real & bandwidth, const std::vector<Real> & data, const MooseEnum & kernelfunction)
{
  Real value = 0;
  if (kernelfunction == "gaussian"){
    for (unsigned i = 0; i < data.size(); ++i){
      value += 1/(data.size() * bandwidth) * Normal::pdf(((x - data[i]) / bandwidth), 0.0, 1.0);
    }
  } else if (kernelfunction == "uniform"){
    for (unsigned i = 0; i < data.size(); ++i){
      value += 1/(data.size() * bandwidth) * Uniform::pdf(((x - data[i]) / bandwidth), -1.0, 1.0);
    }
  }
  return value;
}

Real
KernelDensity1D::cdf(const Real & x, const Real & bandwidth, const std::vector<Real> & data, const MooseEnum & kernelfunction)
{
  Real value = 0;
  if (kernelfunction == "gaussian"){
    for (unsigned i = 0; i < data.size(); ++i){
      value += 1.0/(data.size()) * Normal::cdf(x, data[i], bandwidth);
    }
  } else if (kernelfunction == "uniform"){
    for (unsigned i = 0; i < data.size(); ++i){
      value += 1.0/(data.size()) * Uniform::cdf((x - data[i]) / bandwidth, -1.0, 1.0);
    }
  }
  return value;
}

Real
KernelDensity1D::quantile(const Real & p, const Real & bandwidth, const std::vector<Real> & data, const MooseEnum & kernelfunction)
{
  Real value = 0;
  if (kernelfunction == "gaussian"){
    int index = std::round(p * (data.size()-1));
    value =  (Normal::quantile(p, data[index], bandwidth));
  } else if (kernelfunction == "uniform"){
    int index = std::round(p * (data.size()-1));
    value =  (Uniform::quantile(p, -1.0, 1.0) * bandwidth + data[index]);
  }
  return value;
}

Real
KernelDensity1D::pdf(const Real & x) const
{
  TIME_SECTION(_perf_pdf);
  return pdf(x, _bandwidth, _data, _kernelfunction);
}

Real
KernelDensity1D::cdf(const Real & x) const
{
  TIME_SECTION(_perf_cdf);
  return cdf(x, _bandwidth, _data, _kernelfunction);
}

Real
KernelDensity1D::quantile(const Real & p) const
{
  TIME_SECTION(_perf_quantile);
  return quantile(p, _bandwidth, _data, _kernelfunction);
}
