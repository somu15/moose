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
#include "AdaptiveMonteCarloUtils.h"

registerMooseObjectAliased("StochasticToolsApp", AdaptiveMonteCarloDecision, "AdaptiveMonteCarloDecision");
registerMooseObjectReplaced("StochasticToolsApp",
                            AdaptiveMonteCarloDecision,
                            "07/01/2020 00:00",
                            AdaptiveMonteCarloDecision);

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

// AdaptiveMonteCarloDecision::AdaptiveMonteCarloDecision(const InputParameters & parameters)
//   : GeneralReporter(parameters),
//     _output(declareAdaptiveMonteCarloDecisionValues<Real>("output")),
//     _inputs(declareAdaptiveMonteCarloDecisionValues<Real>("inputs")),
//     _step(getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")->timeStep())
// {
//   _sampler = &getSamplerByName(getParam<SamplerName>("sampler"));
//   _inputs_sto.resize(_sampler->parameters().get<std::vector<DistributionName>>("distributions").size());
//   if (_sampler->parameters().get<std::string>("_type") == "SubsetSimulation")
//   {
//     _inputs_sorted.resize(_sampler->parameters().get<std::vector<DistributionName>>("distributions").size());
//     _subset = 0;
//     _count = 0;
//     _check_even = 0;
//   } else if(_sampler->parameters().get<std::string>("_type") == "AIS")
//   {
//     _prev_val.resize(_sampler->parameters().get<std::vector<DistributionName>>("distributions").size());
//     for (unsigned int i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
//     {
//       _prev_val[i].resize(_sampler->parameters().get<dof_id_type>("num_rows"));
//       _prev_val[i][0] = _sampler->parameters().get<std::vector<Real>>("seeds")[i];
//     }
//     _prev_val_out.resize(1);
//     _prev_val_out[0] = 1.0;
//   }
// }

AdaptiveMonteCarloDecision::AdaptiveMonteCarloDecision(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _output(declareAdaptiveMonteCarloDecisionValues<Real>("output")),
    _inputs(declareAdaptiveMonteCarloDecisionValues<Real>("inputs")),
    _step(getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")->timeStep())
{
  // _sampler = &getSamplerByName(getParam<SamplerName>("sampler"));
  // _inputs_sto.resize(_sampler->parameters().get<std::vector<DistributionName>>("distributions").size());
  // if (_sampler->parameters().get<std::string>("_type") == "SubsetSimulation")
  // {
  //   _inputs_sorted.resize(_sampler->parameters().get<std::vector<DistributionName>>("distributions").size());
  //   _subset = 0;
  //   _count = 0;
  //   _check_even = 0;
  //   for (unsigned int i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
  //   {
  //     _inputs_sto[i].resize(_sampler->parameters().get<dof_id_type>("num_rows"));
  //     _inputs_sorted[i].resize(_sampler->parameters().get<dof_id_type>("num_rows"));
  //   }
  //   _outputs_sto.resize(_sampler->parameters().get<dof_id_type>("num_rows"));
  //   _output_sorted.resize(_sampler->parameters().get<dof_id_type>("num_rows"));
  //   // for (unsigned int i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
  //   //   _inputs_sto[i].resize(_sampler->parameters().get<dof_id_type>("num_rows"));
  // } else if(_sampler->parameters().get<std::string>("_type") == "AIS")
  // {
  //   _prev_val.resize(_sampler->parameters().get<std::vector<DistributionName>>("distributions").size());
  //   for (unsigned int i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
  //   {
  //     _prev_val[i].resize(_sampler->parameters().get<dof_id_type>("num_rows"));
  //     for (unsigned int j = 0; j < _sampler->parameters().get<dof_id_type>("num_rows"); ++j)
  //       _prev_val[i][j] = _sampler->parameters().get<std::vector<Real>>("seeds")[i];
  //   }
  //   _prev_val_out.resize(_sampler->parameters().get<dof_id_type>("num_rows"));
  //   for (unsigned int j = 0; j < _sampler->parameters().get<dof_id_type>("num_rows"); ++j)
  //     _prev_val_out[j] = 1.0;
  // }

  _sampler = &getSamplerByName(getParam<SamplerName>("sampler"));
  _inputs_sto.resize(_sampler->parameters().get<std::vector<DistributionName>>("distributions").size());
  if (_sampler->parameters().get<std::string>("_type") == "SubsetSimulation")
  {
    _inputs_sorted.resize(_sampler->parameters().get<std::vector<DistributionName>>("distributions").size());
    _subset = 0;
    _count = 0;
    _check_even = 0;
    for (unsigned int i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
    {
      _inputs_sto[i].resize(1);
      _inputs_sorted[i].resize(1);
    }
    _outputs_sto.resize(1);
    _output_sorted.resize(1);
    // for (unsigned int i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
    //   _inputs_sto[i].resize(_sampler->parameters().get<dof_id_type>("num_rows"));
  } else if(_sampler->parameters().get<std::string>("_type") == "AIS")
  {
    _prev_val.resize(_sampler->parameters().get<std::vector<DistributionName>>("distributions").size());
    for (unsigned int i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
    {
      _prev_val[i].resize(1);
      for (unsigned int j = 0; j < 1; ++j)
        _prev_val[i][j] = _sampler->parameters().get<std::vector<Real>>("seeds")[i];
    }
    _prev_val_out.resize(1);
    for (unsigned int j = 0; j < 1; ++j)
      _prev_val_out[j] = 1.0;
  }
}

void
AdaptiveMonteCarloDecision::initialize()
{
}

void
AdaptiveMonteCarloDecision::execute()
{
  if (_sampler->parameters().get<std::string>("_type") == "SubsetSimulation")
  {
    int k = 0;
    if (_step <= (_sampler->parameters().get<int>("num_samplessub")))
    {
      _subset = std::floor(_step / _sampler->parameters().get<int>("num_samplessub"));
      for (dof_id_type i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
      {
        (*_inputs[i]) = _sampler->getNextLocalRow()[i];
        // (*_inputs[i]) = _sampler->getLocalSamples()(k,i);
        _inputs_sto[i][k].push_back((*_inputs[i]));
      }
      // std::cout << (*_inputs[0]) << "," << (*_inputs[1]) << std::endl;
      // DenseMatrix<Real> tmp =  _sampler->getLocalSamples();
      // std::cout << tmp(0,0) << tmp(1,0) << "," << std::endl;
      (*_output[0]) = (_sampler->parameters().get<bool>("use_absolute_value")) ? std::abs(*_output[0]) : (*_output[0]);
      _outputs_sto[k].push_back((*_output[0]));
      // std::cout << "Size is" << _outputs_sto.size() << std::endl;
    } else
    {
      _subset = std::floor((_step-1) / _sampler->parameters().get<int>("num_samplessub"));
      _count_max = std::floor(1 / _sampler->parameters().get<Real>("subset_probability"));
      if (_subset > (std::floor((_step-2) /  _sampler->parameters().get<int>("num_samplessub"))))
      {
        _ind_sto = -1;
        _count = INT_MAX;
        _output_sorted[k] = AdaptiveMonteCarloUtils::sortOUTPUT(_outputs_sto[k], _sampler->parameters().get<int>("num_samplessub"), _subset, _sampler->parameters().get<Real>("subset_probability"));
        for (dof_id_type j = 0; j < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++j)
        {
          _inputs_sorted[j][k].resize(std::floor(_sampler->parameters().get<int>("num_samplessub") * _sampler->parameters().get<Real>("subset_probability")));
          _inputs_sorted[j][k] = AdaptiveMonteCarloUtils::sortINPUT(_inputs_sto[j][k], _outputs_sto[k], _sampler->parameters().get<int>("num_samplessub"), _subset, _sampler->parameters().get<Real>("subset_probability"));
        }
        _output_limits.push_back(AdaptiveMonteCarloUtils::computeMIN(_output_sorted[k]));
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
          // (*_inputs[i]) = _sampler->getLocalSamples()(k,i);
          _inputs_sto[i][k].push_back((*_inputs[i]));
        }
        (*_output[0]) = (_sampler->parameters().get<bool>("use_absolute_value")) ? std::abs(*_output[0]) : (*_output[0]);
        _outputs_sto[k].push_back((*_output[0]));
      } else
      {
        for (dof_id_type i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
        {
          (*_inputs[i]) = _inputs_sorted[i][k][_ind_sto];
          _inputs_sto[i][k].push_back((*_inputs[i]));
        }
        (*_output[0]) = _output_sorted[k][_ind_sto];
        _outputs_sto[k].push_back((*_output[0]));
      }
    }
  } else if (_sampler->parameters().get<std::string>("_type") == "AIS")
  {
    if (_check_even != _step)
    {
      if (_step <= (_sampler->parameters().get<int>("num_samples_train")))
      {
        // std::cout << "Here" << std::endl;
        if ( ((_sampler->parameters().get<bool>("use_absolute_value")) ? std::abs(*_output[0]) : (*_output[0])) < (_sampler->parameters().get<Real>("output_limit")))
        {
          for (dof_id_type i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
          {
            (*_inputs[i]) = _prev_val[i][0];
          }
          (*_output[0]) = 0.0; // _prev_val_out[0];
          // std::cout << "Out is" << (*_output[0]) << std::endl;
        } else
        {
          for (dof_id_type i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
          {
            (*_inputs[i]) = _sampler->getNextLocalRow()[i];
            _prev_val[i][0] = (*_inputs[i]);
          }
          (*_output[0]) = 1.0;
          // std::cout << "Out is" << (*_output[0]) << std::endl;
          _prev_val_out[0] = (*_output[0]);
        }
      } else
      {
        for (dof_id_type i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
          (*_inputs[i]) = _sampler->getNextLocalRow()[i];
        _prev_val_out[0] = (_sampler->parameters().get<bool>("use_absolute_value")) ? std::abs(*_output[0]) : (*_output[0]);
        if (_prev_val_out[0] >= (_sampler->parameters().get<Real>("output_limit")))
          (*_output[0]) = 1.0;
        else
          (*_output[0]) = 0.0;
      }
    }
    _check_even = _step;
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
//       for (dof_id_type k = _sampler->getLocalRowBegin(); k < _sampler->getLocalRowEnd(); ++k)
//       {
//         for (dof_id_type i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
//         {
//           (*_inputs[i]) = _sampler->getLocalSamples()(k,i);
//           _inputs_sto[i][k].push_back((*_inputs[i]));
//         }
//         (*_output[0]) = (_sampler->parameters().get<bool>("use_absolute_value")) ? std::abs(*_output[0]) : (*_output[0]);
//         _outputs_sto[k].push_back((*_output[0]));
//       }
//       // std::cout << (*_inputs[0]) << "," << (*_inputs[1]) << std::endl;
//       // DenseMatrix<Real> tmp =  _sampler->getLocalSamples();
//       // std::cout << tmp(0,0) << tmp(1,0) << "," << std::endl;
//
//     } else
//     {
//       _subset = std::floor((_step-1) / _sampler->parameters().get<int>("num_samplessub"));
//       _count_max = std::floor(1 / _sampler->parameters().get<Real>("subset_probability"));
//       if (_subset > (std::floor((_step-2) /  _sampler->parameters().get<int>("num_samplessub"))))
//       {
//         _ind_sto = -1;
//         _count = INT_MAX;
//         _output_sorted[k] = AdaptiveMonteCarloUtils::sortOUTPUT(_outputs_sto[k], _sampler->parameters().get<int>("num_samplessub"), _subset, _sampler->parameters().get<Real>("subset_probability"));
//         for (dof_id_type j = 0; j < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++j)
//         {
//           _inputs_sorted[j][k].resize(std::floor(_sampler->parameters().get<int>("num_samplessub") * _sampler->parameters().get<Real>("subset_probability")));
//           _inputs_sorted[j][k] = AdaptiveMonteCarloUtils::sortINPUT(_inputs_sto[j][k], _outputs_sto[k], _sampler->parameters().get<int>("num_samplessub"), _subset, _sampler->parameters().get<Real>("subset_probability"));
//         }
//         _output_limits.push_back(AdaptiveMonteCarloUtils::computeMIN(_output_sorted[k]));
//       }
//       if (_count >= _count_max)
//       {
//         ++_ind_sto;
//         _count = 0;
//       }
//       ++_count;
//       if ( ((_sampler->parameters().get<bool>("use_absolute_value")) ? std::abs(*_output[0]) : (*_output[0])) >= _output_limits[_subset-1])
//       {
//         for (dof_id_type k = _sampler->getLocalRowBegin(); k < _sampler->getLocalRowEnd(); ++k)
//         {
//           for (dof_id_type i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
//           {
//             (*_inputs[i]) = _sampler->getLocalSamples()(k,i);
//             _inputs_sto[i][k].push_back((*_inputs[i]));
//           }
//           (*_output[0]) = (_sampler->parameters().get<bool>("use_absolute_value")) ? std::abs(*_output[0]) : (*_output[0]);
//           _outputs_sto[k].push_back((*_output[0]));
//         }
//         // for (dof_id_type i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
//         // {
//         //   (*_inputs[i]) = _sampler->getNextLocalRow()[i];
//         //   _inputs_sto[i][k].push_back((*_inputs[i]));
//         // }
//         // (*_output[0]) = (_sampler->parameters().get<bool>("use_absolute_value")) ? std::abs(*_output[0]) : (*_output[0]);
//         // _outputs_sto[k].push_back((*_output[0]));
//       } else
//       {
//         for (dof_id_type i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
//         {
//           (*_inputs[i]) = _inputs_sorted[i][k][_ind_sto];
//           _inputs_sto[i][k].push_back((*_inputs[i]));
//         }
//         (*_output[0]) = _output_sorted[k][_ind_sto];
//         _outputs_sto[k].push_back((*_output[0]));
//       }
//     }
//   } else if (_sampler->parameters().get<std::string>("_type") == "AIS")
//   {
//     if (_step <= (_sampler->parameters().get<int>("num_samples_train")))
//     {
//       if ( ((_sampler->parameters().get<bool>("use_absolute_value")) ? std::abs(*_output[0]) : (*_output[0])) < (_sampler->parameters().get<Real>("output_limit")))
//       {
//         for (dof_id_type i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
//         {
//           (*_inputs[i]) = _prev_val[i][0];
//         }
//         (*_output[0]) = _prev_val_out[0];
//       } else
//       {
//         for (dof_id_type i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
//         {
//           (*_inputs[i]) = _sampler->getNextLocalRow()[i];
//           _prev_val[i][0] = (*_inputs[i]);
//         }
//         (*_output[0]) = 1.0;
//         _prev_val_out[0] = (*_output[0]);
//       }
//     } else
//     {
//       for (dof_id_type i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
//         (*_inputs[i]) = _sampler->getNextLocalRow()[i];
//       _prev_val_out[0] = (_sampler->parameters().get<bool>("use_absolute_value")) ? std::abs(*_output[0]) : (*_output[0]);
//       if (_prev_val_out[0] >= (_sampler->parameters().get<Real>("output_limit")))
//         (*_output[0]) = 1.0;
//       else
//         (*_output[0]) = 0.0;
//     }
//   }
// }
