//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AdaptiveMonteCarloDecision.h"
#include "Sampler.h"
#include "Normal.h"
#include "Distribution.h"

registerMooseObject("MooseApp", AdaptiveMonteCarloDecision);

InputParameters
AdaptiveMonteCarloDecision::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription("Reporter with constant values to be accessed by other objects, can "
                             "be modified using transfers.");

  params += addReporterTypeParams<Real>("output");
  params += addReporterTypeParams<Real>("inputs");
  params.addRequiredParam<SamplerName>("sampler", "Training set defined by a sampler object.");

  return params;
}

AdaptiveMonteCarloDecision::AdaptiveMonteCarloDecision(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _output(declareAdaptiveMonteCarloDecisionValues<Real>("output")),
    _inputs(declareAdaptiveMonteCarloDecisionValues<Real>("inputs")),
    _step(getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")->timeStep())
{
  _sampler = &getSamplerByName(getParam<SamplerName>("sampler"));
  _inputs_sto.resize(_sampler->parameters().get<std::vector<DistributionName>>("distributions").size());
  if (_sampler->parameters().get<std::string>("_type") == "SubsetSimulation")
  {
    _inputs_sorted.resize(_sampler->parameters().get<std::vector<DistributionName>>("distributions").size());
    _subset = 0;
    _count = 0;
    _check_even = 0;
  } else if(_sampler->parameters().get<std::string>("_type") == "AIS")
  {
    // std::cout << "here" << std::endl;
    _prev_val.resize(_sampler->parameters().get<std::vector<DistributionName>>("distributions").size());
    for (unsigned int i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
    {
      _prev_val[i].resize(_sampler->parameters().get<dof_id_type>("num_rows"));
      _prev_val[i][0] = _sampler->parameters().get<std::vector<Real>>("seeds")[i];
    }
    // std::cout << "Here out" << std::endl;
       // Normal::quantile(_sampler->parameters().get<std::vector<DistributionName>>("distributions")[i]::cdf(_sampler->parameters().get<std::vector<Real>>("seeds")[i]),0,1); //
      // Normal::quantile(_distributions[i]->cdf(_seeds[i]),0,1)
    _prev_val_out.resize(1);
    _prev_val_out[0] = 1.0;
  }
}

void
AdaptiveMonteCarloDecision::initialize()
{
  // _sampler = &getSamplerByName(getParam<SamplerName>("sampler"));
  // _inputs_sto.resize(_sampler->parameters().get<std::vector<DistributionName>>("distributions").size());
  // if (_sampler->parameters().get<std::string>("_type") == "SubsetSimulation")
  // {
  //   _inputs_sorted.resize(_sampler->parameters().get<std::vector<DistributionName>>("distributions").size());
  //   _subset = 0;
  //   _count = 0;
  //   _check_even = 0;
  // } else if(_sampler->parameters().get<std::string>("_type") == "AIS")
  // {
  //   std::cout << "here" << std::endl;
  //   _prev_val.resize(_sampler->parameters().get<std::vector<DistributionName>>("distributions").size());
  //   for (unsigned int i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
  //   {
  //     _prev_val[i].resize(_sampler->parameters().get<dof_id_type>("num_rows"));
  //     _prev_val[i][0] = _sampler->parameters().get<std::vector<Real>>("seeds")[i];
  //   }
  //   // std::cout << "Here out" << std::endl;
  //      // Normal::quantile(_sampler->parameters().get<std::vector<DistributionName>>("distributions")[i]::cdf(_sampler->parameters().get<std::vector<Real>>("seeds")[i]),0,1); //
  //     // Normal::quantile(_distributions[i]->cdf(_seeds[i]),0,1)
  //   _prev_val_out.resize(1);
  //   _prev_val_out[0] = 1.0;
  // }


  // {
  //   for (unsigned int i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
  //     _inputs_sto[i].push_back(Normal::quantile(_distributions[i]->cdf(_seeds[i]),0,1));
  // }
}

std::vector<Real>
AdaptiveMonteCarloDecision::sortOUTPUT(const std::vector<Real> & outputs, const int & samplessub, const unsigned int & subset, const Real & subset_prob)
{
  std::vector<Real> tmp;
  tmp.resize(samplessub);
  for (unsigned int i = ((subset-1) * samplessub); i < (subset * samplessub); ++i)
  {
    tmp[i - ((subset-1) * samplessub)] = (outputs[i]);
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
AdaptiveMonteCarloDecision::sortINPUT(const std::vector<Real> & inputs, const std::vector<Real> & outputs, const int & samplessub, const unsigned int & subset, const Real & subset_prob)
{
  std::vector<Real> tmp;
  std::vector<Real> tmp1;
  tmp.resize(samplessub);
  tmp1.resize(samplessub);
  for (unsigned int i = ((subset-1) * samplessub); i < (subset * samplessub); ++i)
  {
    tmp[i - ((subset-1) * samplessub)] = (outputs[i]);
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
AdaptiveMonteCarloDecision::computeMIN(const std::vector<Real> & data)
{
  Real tmp = data[0];
  for (unsigned int i = 0; i < data.size(); ++i)
  {
    if(tmp>(data[i]))
    {
       tmp=(data[i]);
    }
  }
  return tmp;
}

void
AdaptiveMonteCarloDecision::execute()
{
  if (_sampler->parameters().get<std::string>("_type") == "SubsetSimulation")
  {
    if (_step <= (_sampler->parameters().get<int>("num_samplessub")))
    {
      _subset = std::floor(_step / _sampler->parameters().get<int>("num_samplessub"));
      for (dof_id_type i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
      {
        (*_inputs[i]) = _sampler->getNextLocalRow()[i];
        _inputs_sto[i].push_back((*_inputs[i]));
      }
      (*_output[0]) = (_sampler->parameters().get<bool>("use_absolute_value")) ? std::abs(*_output[0]) : (*_output[0]);
      _outputs_sto.push_back((*_output[0]));
    } else
    {
      _subset = std::floor((_step-1) / _sampler->parameters().get<int>("num_samplessub"));
      _count_max = std::floor(1 / _sampler->parameters().get<Real>("subset_probability"));
      if (_subset > (std::floor((_step-2) /  _sampler->parameters().get<int>("num_samplessub"))))
      {
        _ind_sto = -1;
        _count = INT_MAX;
        _output_sorted = sortOUTPUT(_outputs_sto, _sampler->parameters().get<int>("num_samplessub"), _subset, _sampler->parameters().get<Real>("subset_probability"));
        for (dof_id_type j = 0; j < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++j)
        {
          _inputs_sorted[j].resize(std::floor(_sampler->parameters().get<int>("num_samplessub") * _sampler->parameters().get<Real>("subset_probability")));
          _inputs_sorted[j] = sortINPUT(_inputs_sto[j], _outputs_sto, _sampler->parameters().get<int>("num_samplessub"), _subset, _sampler->parameters().get<Real>("subset_probability"));
        }
        _output_limits.push_back(computeMIN(_output_sorted));
      }
      if (_count >= _count_max)
      {
        ++_ind_sto;
        _count = 0;
      }
      ++_count;
      if ( ((_sampler->parameters().get<bool>("use_absolute_value")) ? std::abs(*_output[0]) : (*_output[0])) >= _output_limits[_subset-1])
      {
        for (dof_id_type i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
        {
          (*_inputs[i]) = _sampler->getNextLocalRow()[i];
          _inputs_sto[i].push_back((*_inputs[i]));
        }
        (*_output[0]) = (_sampler->parameters().get<bool>("use_absolute_value")) ? std::abs(*_output[0]) : (*_output[0]);
        _outputs_sto.push_back((*_output[0]));
      } else
      {
        for (dof_id_type i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
        {
          (*_inputs[i]) = _inputs_sorted[i][_ind_sto];
          _inputs_sto[i].push_back((*_inputs[i]));
        }
        (*_output[0]) = _output_sorted[_ind_sto];
        _outputs_sto.push_back((*_output[0]));
      }
    }
  } else if (_sampler->parameters().get<std::string>("_type") == "AIS")
  {
    // std::cout << "Value is " << _sampler->getNextLocalRow()[0] << std::endl;
    // std::cout << "Here" << std::endl;
    if (_step <= (_sampler->parameters().get<int>("num_samples_train")))
    {
      // std::cout << "Value is " << _prev_val[0][0] << std::endl;
      // std::cout << "Here 0" << std::endl;
      if ( ((_sampler->parameters().get<bool>("use_absolute_value")) ? std::abs(*_output[0]) : (*_output[0])) < (_sampler->parameters().get<Real>("output_limit")))
      {
        // std::cout << "Value is " << _sampler->getNextLocalRow()[0] << std::endl;
        // std::cout << "Here 1" << std::endl;
        for (dof_id_type i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
        {
          (*_inputs[i]) = _prev_val[i][0];
        }
        (*_output[0]) = _prev_val_out[0];
      } else
      {
        // std::cout << "Here 2" << std::endl;
        for (dof_id_type i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
        {
          (*_inputs[i]) = _sampler->getNextLocalRow()[i]; // Normal::quantile(_sampler->parameters().get<std::vector<DistributionName>>("distributions")[i]->cdf(_sampler->getNextLocalRow()[i]),0,1); //
          _prev_val[i][0] = (*_inputs[i]);
        }
        (*_output[0]) = 1.0; // (_sampler->parameters().get<bool>("use_absolute_value")) ? std::abs(*_output[0]) : (*_output[0]);
        _prev_val_out[0] = (*_output[0]);
      }
    } else
    {
      for (dof_id_type i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
        (*_inputs[i]) = _sampler->getNextLocalRow()[i];
      // for (dof_id_type i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
      //   (*_inputs[i]) = Normal::quantile(_sampler->parameters().get<std::vector<DistributionName>>("distributions")[i]->cdf(_sampler->getNextLocalRow()[i]),0,1); // _sampler->getNextLocalRow()[i];
      _prev_val_out[0] = (_sampler->parameters().get<bool>("use_absolute_value")) ? std::abs(*_output[0]) : (*_output[0]);
      if (_prev_val_out[0] >= (_sampler->parameters().get<Real>("output_limit")))
        (*_output[0]) = 1.0;
      else
        (*_output[0]) = 0.0;
    }
  }
}

// void
// AdaptiveMonteCarloDecision::execute()
// {
//   if (_sampler->parameters().get<std::string>("_type") == "SubsetSimulation")
//   {
//     if (_step <= (_sampler->parameters().get<int>("num_samplessub")))
//     {
//       _subset = std::floor(_step / _sampler->parameters().get<int>("num_samplessub"));
//       for (dof_id_type i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
//       {
//         (*_inputs[i]) = _sampler->getNextLocalRow()[i];
//         _inputs_sto[i].push_back((*_inputs[i]));
//       }
//       (*_output[0]) = (_sampler->parameters().get<bool>("use_absolute_value")) ? std::abs(*_output[0]) : (*_output[0]);
//       _outputs_sto.push_back((*_output[0]));
//     } else
//     {
//       if (_check_even != _step)
//       {
//         _subset = std::floor((_step-1) / _sampler->parameters().get<int>("num_samplessub"));
//         _count_max = std::floor(1 / _sampler->parameters().get<Real>("subset_probability"));
//         if (_subset > (std::floor((_step-2) /  _sampler->parameters().get<int>("num_samplessub"))))
//         {
//           _ind_sto = -1;
//           _count = INT_MAX;
//           _output_sorted = sortOUTPUT(_outputs_sto, _sampler->parameters().get<int>("num_samplessub"), _subset, _sampler->parameters().get<Real>("subset_probability"));
//           for (dof_id_type j = 0; j < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++j)
//           {
//             _inputs_sorted[j].resize(std::floor(_sampler->parameters().get<int>("num_samplessub") * _sampler->parameters().get<Real>("subset_probability")));
//             _inputs_sorted[j] = sortINPUT(_inputs_sto[j], _outputs_sto, _sampler->parameters().get<int>("num_samplessub"), _subset, _sampler->parameters().get<Real>("subset_probability"));
//           }
//           _output_limits.push_back(computeMIN(_output_sorted));
//         }
//         if (_count >= _count_max)
//         {
//           ++_ind_sto;
//           _count = 0;
//         }
//         ++_count;
//         if ( ((_sampler->parameters().get<bool>("use_absolute_value")) ? std::abs(*_output[0]) : (*_output[0])) >= _output_limits[_subset-1])
//         {
//           for (dof_id_type i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
//           {
//             (*_inputs[i]) = _sampler->getNextLocalRow()[i];
//             _inputs_sto[i].push_back((*_inputs[i]));
//           }
//           (*_output[0]) = (_sampler->parameters().get<bool>("use_absolute_value")) ? std::abs(*_output[0]) : (*_output[0]);
//           _outputs_sto.push_back((*_output[0]));
//         } else
//         {
//           for (dof_id_type i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
//           {
//             (*_inputs[i]) = _inputs_sorted[i][_ind_sto];
//             _inputs_sto[i].push_back((*_inputs[i]));
//           }
//           (*_output[0]) = _output_sorted[_ind_sto];
//           _outputs_sto.push_back((*_output[0]));
//         }
//       }
//       _check_even = _step;
//     }
//   }
//
// }
