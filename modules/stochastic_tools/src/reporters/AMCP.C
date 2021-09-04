//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AMCP.h"
#include "Sampler.h"
#include "Normal.h"
#include "Distribution.h"
#include "AdaptiveMonteCarloUtils.h"

registerMooseObjectAliased("StochasticToolsApp", AMCP, "AMCP");
registerMooseObjectReplaced("StochasticToolsApp",
                            AMCP,
                            "07/01/2020 00:00",
                            AMCP);

InputParameters
AMCP::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription("Reporter with constant values to be accessed by other objects, can "
                             "be modified using transfers.");

  params += addReporterTypeParams<Real>("output"); // std::vector<Real>
  // params.addRequiredParam<ReporterName>(
  //     "output", "Reporter value of response results, can be vpp with <vpp_name>/<vector_name>.");
  params += addReporterTypeParams<Real>("inputs"); // std::vector<Real>
  // params.addRequiredParam<ReporterName>(
  //     "inputs", "Reporter value of response results, can be vpp with <vpp_name>/<vector_name>.");
  params.addRequiredParam<SamplerName>("sampler", "Training set defined by a sampler object.");

  return params;
}

AMCP::AMCP(const InputParameters & parameters)
  : GeneralReporter(parameters),
    // _output(declareAMCPValues<Real>("output", 1)),
    // _inputs(declareAMCPValues<Real>("inputs", 1)),
    _step(getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")->timeStep())
{

  // MultiMooseEnum amcs("subset");
  // _subset_out = declareAMCSStatistics<unsigned int>("subset");
  _sampler = &getSamplerByName(getParam<SamplerName>("sampler"));


  _output = declareAMCPValues<Real>("output", _sampler->parameters().get<dof_id_type>("num_rows"));
  _inputs = declareAMCPValues<Real>("inputs", _sampler->parameters().get<dof_id_type>("num_rows"));

  // _output = declareConstantVectorReporterValues<Real>("output");
  // _inputs = declareConstantVectorReporterValues<Real>("inputs");

  _inputs_sto.resize(_sampler->parameters().get<std::vector<DistributionName>>("distributions").size());
  if (_sampler->parameters().get<std::string>("_type") == "SSP")
  {
    _inputs_sorted.resize(_sampler->parameters().get<std::vector<DistributionName>>("distributions").size());
    _subset = 0;
    _count = 0;
    _check_even = 0;
    // _inputs_sto.resize(_sampler->parameters().get<std::vector<DistributionName>>("distributions").size());
    _inputs_sorted.resize(_sampler->parameters().get<std::vector<DistributionName>>("distributions").size());
    _prev_val.resize(_sampler->parameters().get<std::vector<DistributionName>>("distributions").size());
  } else if(_sampler->parameters().get<std::string>("_type") == "AIS")
  {
    _prev_val.resize(_sampler->parameters().get<std::vector<DistributionName>>("distributions").size());
    for (unsigned int i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
      _prev_val[i] = _sampler->parameters().get<std::vector<Real>>("initial_values")[i];
    _prev_val_out = 1.0;
  }
}

void
AMCP::initialize()
{
}

void
AMCP::execute()
{
  if (_sampler->parameters().get<std::string>("_type") == "SSP")
  {
    // if (_step <= (_sampler->parameters().get<int>("num_samplessub")))
    // {
    //   _subset = std::floor(_step / _sampler->parameters().get<int>("num_samplessub"));
    //   (*_subset_out[0]) = _subset;
    //   for (dof_id_type i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
    //   {
    //     (*_inputs[i]) = _sampler->getNextLocalRow()[i];
    //     _inputs_sto[i].push_back((*_inputs[i]));
    //   }
    //   (*_output[0]) = (_sampler->parameters().get<bool>("use_absolute_value")) ? std::abs(*_output[0]) : (*_output[0]);
    //   _outputs_sto.push_back((*_output[0]));
    // }

    if (_step <= (_sampler->parameters().get<int>("num_samplessub")))
    {
      _subset = std::floor(_step / _sampler->parameters().get<int>("num_samplessub"));
      // (*_subset_out[0]) = _subset;
      std::cout << _sampler->getLocalRowEnd() << std::endl;

      for (dof_id_type ss = 0; ss < _sampler->getLocalRowEnd(); ++ss)
      {
        const auto data = _sampler->getNextLocalRow();
        for (dof_id_type i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
          {
            (*_inputs[ss][i]) = data[i];
            _inputs_sto[i].push_back(data[i]);
          }
      }


      for (dof_id_type ss = _sampler->getLocalRowBegin(); ss < _sampler->getLocalRowEnd(); ++ss)
      {
        (*_output[ss][0]) = (*_output[ss][0]); // (_sampler->parameters().get<bool>("use_absolute_value")) ? std::abs((*_output[0][ss])) : (*_output[0][ss]);
        _outputs_sto.push_back((*_output[ss][0]));
      }
    } // else
    // {
    //   _subset = std::floor((_step-1) / _sampler->parameters().get<int>("num_samplessub"));
    //   (*_subset_out[0]) = _subset;
    //   _count_max = std::floor(1 / _sampler->parameters().get<Real>("subset_probability"));
    //   if (_subset > (std::floor((_step-2) /  _sampler->parameters().get<int>("num_samplessub"))))
    //   {
    //     _ind_sto = -1;
    //     _count = INT_MAX;
    //     _output_sorted = AdaptiveMonteCarloUtils::sortOUTPUT(_outputs_sto, _sampler->parameters().get<int>("num_samplessub"), _subset, _sampler->parameters().get<Real>("subset_probability"));
    //     for (dof_id_type j = 0; j < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++j)
    //     {
    //       _inputs_sorted[j].resize(std::floor(_sampler->parameters().get<int>("num_samplessub") * _sampler->parameters().get<Real>("subset_probability")));
    //       _inputs_sorted[j] = AdaptiveMonteCarloUtils::sortINPUT(_inputs_sto[j], _outputs_sto, _sampler->parameters().get<int>("num_samplessub"), _subset, _sampler->parameters().get<Real>("subset_probability"));
    //     }
    //     _output_limits.push_back(AdaptiveMonteCarloUtils::computeMIN(_output_sorted));
    //   }
    //   if (_count >= _count_max)
    //   {
    //     ++_ind_sto;
    //     _count = 0;
    //     // std::cout << "Here" << _ind_sto << std::endl;
    //     for (dof_id_type i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
    //       _prev_val[i] = _inputs_sorted[i][_ind_sto];
    //     _prev_val_out = _output_sorted[_ind_sto];
    //   } else
    //   {
    //     for (dof_id_type i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
    //       _prev_val[i] = _inputs_sto[i][_inputs_sto[i].size()-1];
    //     _prev_val_out = _outputs_sto[_outputs_sto.size()-1];
    //   }
    //   ++_count;
    //   if ( ((_sampler->parameters().get<bool>("use_absolute_value")) ? std::abs(*_output[0]) : (*_output[0])) >= _output_limits[_subset-1])
    //   {
    //     // std::cout << "Accepted" << std::endl;
    //     for (dof_id_type i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
    //     {
    //       (*_inputs[i]) = _sampler->getNextLocalRow()[i];
    //       _inputs_sto[i].push_back((*_inputs[i]));
    //     }
    //     (*_output[0]) = (_sampler->parameters().get<bool>("use_absolute_value")) ? std::abs(*_output[0]) : (*_output[0]);
    //     _outputs_sto.push_back((*_output[0]));
    //   } else
    //   {
    //     for (dof_id_type i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
    //     {
    //       (*_inputs[i]) =  _prev_val[i]; // _inputs_sorted[i][_ind_sto];
    //       _inputs_sto[i].push_back((*_inputs[i]));
    //     }
    //     (*_output[0]) = _prev_val_out; // _output_sorted[_ind_sto];
    //     _outputs_sto.push_back((*_output[0]));
    //   }
    // }
  } // else if (_sampler->parameters().get<std::string>("_type") == "AIS")
  // {
  //   if (_check_even != _step)
  //   {
  //     if (_step <= (_sampler->parameters().get<int>("num_samples_train")))
  //     {
  //       if ( ((_sampler->parameters().get<bool>("use_absolute_value")) ? std::abs(*_output[0]) : (*_output[0])) < (_sampler->parameters().get<Real>("output_limit")))
  //       {
  //         for (dof_id_type i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
  //         {
  //           (*_inputs[i]) = _prev_val[i];
  //         }
  //         (*_output[0]) = 0.0;
  //       } else
  //       {
  //         for (dof_id_type i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
  //         {
  //           (*_inputs[i]) = _sampler->getNextLocalRow()[i];
  //           _prev_val[i] = (*_inputs[i]);
  //         }
  //         (*_output[0]) = 1.0;
  //         _prev_val_out = (*_output[0]);
  //       }
  //     } else
  //     {
  //       for (dof_id_type i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
  //         (*_inputs[i]) = _sampler->getNextLocalRow()[i];
  //       _prev_val_out = (_sampler->parameters().get<bool>("use_absolute_value")) ? std::abs(*_output[0]) : (*_output[0]);
  //       if (_prev_val_out >= (_sampler->parameters().get<Real>("output_limit")))
  //         (*_output[0]) = 1.0;
  //       else
  //         (*_output[0]) =  0.0;
  //     }
  //   }
  //   _check_even = _step;
  // }
}

// template <typename T>
// std::vector<T *>
// AMCP::declareAMCSStatistics(const std::string & statistics)
// {
//   std::vector<T *> data;
//   data.push_back(&this->declareValueByName<T>(statistics, 0));
//   return data;
// }
