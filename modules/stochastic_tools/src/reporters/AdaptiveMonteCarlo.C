//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AdaptiveMonteCarlo.h"
#include "Sampler.h"

registerMooseObject("MooseApp", AdaptiveMonteCarlo);

InputParameters
AdaptiveMonteCarlo::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription("Reporter with constant values to be accessed by other objects, can "
                             "be modified using transfers.");

  params += addReporterTypeParams<int>("integer");
  params += addReporterTypeParams<Real>("real");
  params += addReporterTypeParams<Real>("real_vec");
  // params += addReporterTypeParams<Real>("inputs");
  params += addReporterTypeParams<std::string>("string");
  params.addRequiredParam<SamplerName>("sampler", "Training set defined by a sampler object.");

  return params;
}

AdaptiveMonteCarlo::AdaptiveMonteCarlo(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _int(declareAdaptiveMonteCarloValues<int>("integer")),
    _real(declareAdaptiveMonteCarloValues<Real>("real")),
    // _inputs(declareAdaptiveMonteCarloValues<Real>("inputs")),
    _string(declareAdaptiveMonteCarloValues<std::string>("string")),
    _int_vec(declareConstantVectorReporterValues<int>("integer")),
    _real_vec(declareConstantVectorReporterValues<Real>("real")),
    _string_vec(declareConstantVectorReporterValues<std::string>("string")),
    _step(getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")->timeStep())
{
  _subset = 0;
  _count = 0;
}

void
AdaptiveMonteCarlo::initialize()
{
  _sampler = &getSamplerByName(getParam<SamplerName>("sampler"));
  _inputs_sto.resize(_sampler->parameters().get<std::vector<DistributionName>>("distributions").size());
  _inputs_sorted.resize(_sampler->parameters().get<std::vector<DistributionName>>("distributions").size());
}

std::vector<Real>
AdaptiveMonteCarlo::sortOUTPUT(const std::vector<Real> & outputs, const int & samplessub, const unsigned int & subset, const Real & subset_prob)
{
  std::vector<Real> tmp;
  tmp.resize(samplessub);
  for (unsigned int i = ((subset-1) * samplessub); i < (subset * samplessub); ++i)
  {
    tmp[i - ((subset-1) * samplessub)] = outputs[i];
  }
  std::sort(tmp.begin(), tmp.end());
  std::vector<Real> out;
  out.resize(std::floor(samplessub * subset_prob));
  for (unsigned int i = 0; i < (out.size()); ++i)
  {
    out[i] = tmp[i + std::round(samplessub * (1-subset_prob))];
  }
  return out;
}

std::vector<Real>
AdaptiveMonteCarlo::sortINPUT(const std::vector<Real> & inputs, const std::vector<Real> & outputs, const int & samplessub, const unsigned int & subset, const Real & subset_prob)
{
  std::vector<Real> tmp;
  std::vector<Real> tmp1;
  tmp.resize(samplessub);
  tmp1.resize(samplessub);
  for (unsigned int i = ((subset-1) * samplessub); i < (subset * samplessub); ++i)
  {
    tmp[i - ((subset-1) * samplessub)] = outputs[i];
    tmp1[i - ((subset-1) * samplessub)] = inputs[i];
  }
  std::vector<int> tmp2(tmp.size());
  std::iota(tmp2.begin(), tmp2.end(), 0);
  auto comparator = [&tmp](int a, int b){ return tmp[a] < tmp[b]; };
  std::sort(tmp2.begin(), tmp2.end(), comparator);
  std::vector<Real> out;
  out.resize(std::floor(samplessub * subset_prob));
  for (unsigned int i = 0; i < out.size(); ++i)
  {
    out[i] = tmp1[tmp2[i + std::ceil(samplessub * (1-subset_prob))]];
  }
  return out;
}

Real
AdaptiveMonteCarlo::computeMIN(const std::vector<Real> & data)
{
  Real tmp = data[0];
  for (unsigned int i = 0; i < data.size(); ++i)
  {
    if(tmp>data[i])
    {
       tmp=data[i];
    }
  }
  return tmp;
}

void
AdaptiveMonteCarlo::execute()
{
  if (_sampler->parameters().get<std::string>("_type") == "SubTest")
  {
    _subset = std::floor(_step / _sampler->parameters().get<int>("num_samplessub"));
    if (_step <= _sampler->parameters().get<int>("num_samplessub"))
    {
      std::cout << "Here 0" << std::endl;
      std::cout << _sampler->getNextLocalRow()[0] << "," << _sampler->getNextLocalRow()[1] << std::endl;
      (*_real_vec[0])[0] = _sampler->getNextLocalRow()[0];
      std::cout << "Here 1" << std::endl;
      std::cout << (*_real_vec[0])[0] << std::endl;
      (*_real_vec[0]) = _sampler->getNextLocalRow();
      std::cout << "Here 2" << std::endl;
      for (dof_id_type i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
        _inputs_sto[i].push_back((*_real_vec[0])[i]);
      _outputs_sto.push_back((*_real[0]));
    } else
    {
      _count_max = std::floor(1 / _sampler->parameters().get<Real>("subset_probability"));
      if (_subset > (std::floor((_step-1) /  _sampler->parameters().get<int>("num_samplessub"))))
      {
        _ind_sto = -1;
        _count =  std::numeric_limits<Real>::infinity();
        _output_sorted = sortOUTPUT(_outputs_sto, _sampler->parameters().get<int>("num_samplessub"), _subset, _sampler->parameters().get<Real>("subset_probability"));
        for (dof_id_type j = 0; j < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++j)
        {
          _inputs_sorted[j].resize(std::floor(_sampler->parameters().get<int>("num_samplessub") * _sampler->parameters().get<Real>("subset_probability")));
          _inputs_sorted[j] = sortINPUT(_inputs_sto[j], _outputs_sto, _sampler->parameters().get<int>("num_samplessub"), _subset, _sampler->parameters().get<Real>("subset_probability"));
        }
        _output_limits.push_back(computeMIN(_output_sorted));
      }
      if (_count > _count_max)
      {
        ++_ind_sto;
        _count = 0;
      }
      ++_count;
      if ( (*_real[0]) >= _output_limits[_subset-1])
      {
        for (dof_id_type i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
        {
          (*_real_vec[0]) = _sampler->getNextLocalRow();
          _inputs_sto[i].push_back((*_real_vec[0])[i]);
        }
        _outputs_sto.push_back((*_real[0]));
      } else
      {
        for (dof_id_type i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
        {
          (*_real_vec[0])[i] = _inputs_sorted[i][_ind_sto];
          _inputs_sto[i].push_back(_inputs_sorted[i][_ind_sto]);
        }
        _outputs_sto.push_back(_output_sorted[_ind_sto]);
      }
    }
  }


  // std::cout << "Here" << std::endl;
  // std::cout << _sampler->parameters().get<Real>("subset_probability") << std::endl;
  // std::cout << _sampler->parameters().get<std::string>("_type") << std::endl;
  // std::cout << _num_samplessub << std::endl;
  // std::cout << (*_real[0]) << std::endl;
  //
  // (*_real[0]) = 1.0;
  // Real x = 1.0;
  // _real[0] = &x;
}
