//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AMCP_1.h"
#include "Sampler.h"
#include "Normal.h"
#include "Distribution.h"
#include "AdaptiveMonteCarloUtils.h"
#include "ReporterContext.h"

registerMooseObjectAliased("StochasticToolsApp", AMCP_1, "AMCP_1");
registerMooseObjectReplaced("StochasticToolsApp",
                            AMCP_1,
                            "07/01/2020 00:00",
                            AMCP_1);

InputParameters
AMCP_1::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription("Reporter with constant values to be accessed by other objects, can "
                             "be modified using transfers.");

 // params.addParam<ReporterValueName>(
 //     "output", "output", "The name of the interger data"); // MooseDocs:data
 params.addRequiredParam<ReporterName>(
     "output", "The name of the interger data"); // MooseDocs:data
 // params += addReporterTypeParams<Real>("data");
  // params.addParam<ReporterValueName>(
  //    "output", "output", "The name of the interger data"); // MooseDocs:data
 params.addParam<ReporterValueName>(
     "inputs", "inputs", "The name of the interger data"); // MooseDocs:data
 params.addParam<ReporterValueName>(
     "data", "data", "The name of the interger data"); // MooseDocs:data
  // params.addRequiredParam<ReporterName>(
  //     "output", "Reporter value of response results, can be vpp with <vpp_name>/<vector_name>.");
  // params += addReporterTypeParams<Real>("inputs"); // std::vector<Real>
  // params.addRequiredParam<ReporterName>(
  //     "inputs", "Reporter value of response results, can be vpp with <vpp_name>/<vector_name>.");
  params.addRequiredParam<SamplerName>("sampler", "Training set defined by a sampler object.");

  return params;
}

AMCP_1::AMCP_1(const InputParameters & parameters)
  : GeneralReporter(parameters),
    // _output(declareAMCP_1Values<Real>("output")),
    _output(&getReporterValue<std::vector<Real>>("output", REPORTER_MODE_DISTRIBUTED)),
    // _output(&declareValueByName<std::vector<Real>>("output", REPORTER_MODE_DISTRIBUTED)),
    // _output(declareValue<std::vector<Real>>("output", ReporterGatherContext>("gather"))), // , REPORTER_MODE_REPLICATED
    // _output(declareValue<Real>("output", REPORTER_MODE_DISTRIBUTED)), //
    // _output(declareValueByName<Real>("output", REPORTER_MODE_DISTRIBUTED)), //
    _data(declareValueByName<std::vector<Real>, ReporterGatherContext>("data")),
    // _data_in(declareValueByName<std::vector<std::vector<Real>>>("data_in")), // , ReporterGatherContext
    _data_in(declareValueByName<std::vector<Real>, ReporterGatherContext>("data_in")),
    _inputs(declareValueByName<std::vector<std::vector<Real>>>("inputs", REPORTER_MODE_DISTRIBUTED)),
    // _inputs(declareValueByName<std::vector<std::vector<Real>>, ReporterGatherContext>("inputs")), //
    _step(getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")->timeStep())
{

  // MultiMooseEnum amcs("subset");
  // _subset_out = declareAMCSStatistics<unsigned int>("subset");
  _sampler = &getSamplerByName(getParam<SamplerName>("sampler"));

  // _Tmp.resize(n_processors());

  // _output = declareAMCP_1Values<Real>("output", _sampler->parameters().get<dof_id_type>("num_rows"));
  // _inputs = declareAMCP_1Values<Real>("inputs", _sampler->parameters().get<dof_id_type>("num_rows"));

  // _output = declareConstantVectorReporterValues<Real>("output");
  // _inputs = declareConstantVectorReporterValues<Real>("inputs");

  // _output.resize(1);

  _inputs_sto.resize(_sampler->parameters().get<std::vector<DistributionName>>("distributions").size());
  _inputs.resize(_sampler->parameters().get<std::vector<DistributionName>>("distributions").size());

  // _inputs_sto.resize(_sampler->getNumberOfRows());

  // _data_in->resize(_sampler->parameters().get<std::vector<DistributionName>>("distributions").size());
  for (dof_id_type i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
  {
    _inputs[i].resize(_sampler->getNumberOfRows());
  }

  // if (processor_id() == 0)
  // {
  //
  // }
  // _data.resize(_sampler->getNumberOfRows());
  _data_in.resize(_sampler->getNumberOfRows());
  // std::cout << (*_data_in)[0] << std::endl;
  // for (dof_id_type i = 0; i < _sampler->getNumberOfRows(); ++i)
  //   _data_in[i].resize(_sampler->parameters().get<std::vector<DistributionName>>("distributions").size());



  // if (processor_id() == 0)
  // {
  //
  // }

  if (_sampler->parameters().get<std::string>("_type") == "SSP_1")
  {
    _inputs_sorted.resize(_sampler->parameters().get<std::vector<DistributionName>>("distributions").size());
    _subset = 0;
    _count = 0;
    _check_even = 0;
    // _inputs_sto.resize(_sampler->parameters().get<std::vector<DistributionName>>("distributions").size());
    _inputs_sorted.resize(_sampler->parameters().get<std::vector<DistributionName>>("distributions").size());
    _prev_val.resize(_sampler->parameters().get<std::vector<DistributionName>>("distributions").size());
    for (dof_id_type j = 0; j < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++j)
      _prev_val[j].resize(n_processors());
    _prev_val_out.resize(n_processors());
  }
  // } else if(_sampler->parameters().get<std::string>("_type") == "AIS")
  // {
  //   _prev_val.resize(_sampler->parameters().get<std::vector<DistributionName>>("distributions").size());
  //   for (unsigned int i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
  //     _prev_val[i] = _sampler->parameters().get<std::vector<Real>>("initial_values")[i];
  //   _prev_val_out = 1.0;
  // }
}

void
AMCP_1::initialize()
{
}

// void
// AMCP_1::execute()
// {
//   if (_sampler->parameters().get<std::string>("_type") == "SSP_1")
//   {
//     if (_step <= (_sampler->parameters().get<int>("num_samplessub") / n_processors()))
//     {
//       _subset = std::floor((_step * n_processors()) / _sampler->parameters().get<int>("num_samplessub"));
//       for (dof_id_type ss = _sampler->getLocalRowBegin(); ss < _sampler->getLocalRowEnd(); ++ss)
//       {
//         const auto data = _sampler->getNextLocalRow();
//         _data_in = data;
//         _communicator.allgather(_data_in);
//       }
//       for (dof_id_type ss = 0; ss < (_sampler->getNumberOfRows()); ++ss)
//       {
//         for (dof_id_type i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
//         {
//           _inputs_sto[i].push_back(_data_in[(_sampler->parameters().get<std::vector<DistributionName>>("distributions").size())*ss+i]);
//           _inputs[i][ss] = _data_in[(_sampler->parameters().get<std::vector<DistributionName>>("distributions").size())*ss+i];
//         }
//       }
//       // std::cerr << "Here" << std::endl;
//       // std::cerr << Moose::stringify((*_output)) << std::endl; // Moose::stringify((*_output))  (*_output)[0]
//       _data = (_sampler->parameters().get<bool>("use_absolute_value")) ? AdaptiveMonteCarloUtils::computeABS((*_output)) : (*_output); // (*_output);
//       // (*_output) = _data; // (_sampler->parameters().get<bool>("use_absolute_value")) ? AdaptiveMonteCarloUtils::computeABS((_output)) : (_output); //
//       _communicator.allgather(_data);
//       // std::cerr << Moose::stringify(_data) << std::endl;
//       for (dof_id_type ss = 0; ss < _data.size(); ++ss)
//         _outputs_sto.push_back(_data[ss]);
//     } else
//     {
//       _subset = std::floor(((_step-1) * n_processors()) / _sampler->parameters().get<int>("num_samplessub"));
//       _count_max = std::floor(1 / _sampler->parameters().get<Real>("subset_probability"));
//       if (_subset > (std::floor(((_step-2) * n_processors()) /  _sampler->parameters().get<int>("num_samplessub"))))
//       {
//         _ind_sto = -1;
//         _count = INT_MAX;
//         _output_sorted = AdaptiveMonteCarloUtils::sortOUTPUT(_outputs_sto, _sampler->parameters().get<int>("num_samplessub"), _subset, _sampler->parameters().get<Real>("subset_probability"));
//         for (dof_id_type j = 0; j < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++j)
//         {
//           _inputs_sorted[j].resize(std::floor(_sampler->parameters().get<int>("num_samplessub") * _sampler->parameters().get<Real>("subset_probability")));
//           _inputs_sorted[j] = AdaptiveMonteCarloUtils::sortINPUT(_inputs_sto[j], _outputs_sto, _sampler->parameters().get<int>("num_samplessub"), _subset, _sampler->parameters().get<Real>("subset_probability"));
//         }
//         _output_limits.push_back(AdaptiveMonteCarloUtils::computeMIN(_output_sorted));
//       }
//       if (_count >= _count_max)
//       {
//         for (dof_id_type jj = 0; jj < n_processors(); ++jj)
//         {
//           ++_ind_sto;
//           for (dof_id_type k = 0; k < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++k)
//             _prev_val[k][jj] = _inputs_sorted[k][_ind_sto];
//           _prev_val_out[jj] = _output_sorted[_ind_sto];
//         }
//         _count = 0;
//       } else
//       {
//         for (dof_id_type jj = 0; jj < n_processors(); ++jj)
//         {
//           for (dof_id_type k = 0; k < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++k)
//             _prev_val[k][jj] = _inputs_sto[k][_inputs_sto[k].size()-n_processors()+jj];
//           _prev_val_out[jj] = _outputs_sto[_outputs_sto.size()-n_processors()+jj];
//         }
//       }
//       ++_count;
//       for (dof_id_type ss = _sampler->getLocalRowBegin(); ss < _sampler->getLocalRowEnd(); ++ss)
//       {
//         const auto data = _sampler->getNextLocalRow();
//         _data_in = data;
//         _communicator.allgather(_data_in);
//       }
//       _data = (_sampler->parameters().get<bool>("use_absolute_value")) ? AdaptiveMonteCarloUtils::computeABS((*_output)) :  (*_output); //;
//       _communicator.allgather(_data);
//       std::vector<Real> Tmp2 = _data;
//       // std::cerr << Moose::stringify(_data) << std::endl;
//       for (dof_id_type ss = 0; ss < n_processors(); ++ss)
//       {
//         if (Tmp2[ss] >= _output_limits[_subset-1])
//         {
//           for (dof_id_type i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
//           {
//             _inputs[i][ss] = _data_in[(_sampler->parameters().get<std::vector<DistributionName>>("distributions").size())*ss+i];
//             _inputs_sto[i].push_back(_inputs[i][ss]);
//           }
//           // (_output[ss]) = _data[ss];
//           _outputs_sto.push_back(Tmp2[ss]);
//         } else
//         {
//           for (dof_id_type i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
//           {
//             _inputs[i][ss] = _prev_val[i][ss];
//             _data_in[(_sampler->parameters().get<std::vector<DistributionName>>("distributions").size())*ss+i] = _inputs[i][ss];
//             _inputs_sto[i].push_back(_inputs[i][ss]);
//           }
//           Tmp2[ss] = _prev_val_out[ss];
//           // (_output[ss]) = _prev_val_out[ss];
//           _outputs_sto.push_back(Tmp2[ss]);
//         }
//         //
//       }
//       _data = Tmp2;
//       // std::cerr << Moose::stringify(_output_limits) << std::endl;
//       // std::cerr << Moose::stringify(_outputs_sto) << std::endl;
//     }
//
//
//
//
//     // _subset = std::floor(_step / _sampler->parameters().get<int>("num_samplessub"));
//     //
//     // for (dof_id_type ss = _sampler->getLocalRowBegin(); ss < _sampler->getLocalRowEnd(); ++ss)
//     // {
//     //   const auto data = _sampler->getNextLocalRow();
//     //   _data_in = data;
//     //   _communicator.allgather(_data_in);
//     // }
//     //
//     // for (dof_id_type ss = 0; ss < (_sampler->getNumberOfRows()); ++ss)
//     // {
//     //   for (dof_id_type i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
//     //   {
//     //     _inputs_sto[i].push_back(_data_in[(_sampler->parameters().get<std::vector<DistributionName>>("distributions").size())*ss+i]);
//     //     _inputs[i][ss] = _data_in[(_sampler->parameters().get<std::vector<DistributionName>>("distributions").size())*ss+i];
//     //   }
//     // }
//     //
//     // _data = (*_output);
//     // _communicator.allgather(_data);
//     // for (dof_id_type ss = 0; ss < _data.size(); ++ss)
//     //   _outputs_sto.push_back(_data[ss]);
//
//   }
// }

void
AMCP_1::execute()
{
  if (_sampler->parameters().get<std::string>("_type") == "SSP_1")
  {
    if (_step <= (_sampler->parameters().get<int>("num_samplessub") / n_processors()))
    {
      _subset = std::floor((_step * n_processors()) / _sampler->parameters().get<int>("num_samplessub"));
      for (dof_id_type ss = _sampler->getLocalRowBegin(); ss < _sampler->getLocalRowEnd(); ++ss)
      {
        const auto data = _sampler->getNextLocalRow();
        _data_in = data;
        _communicator.allgather(_data_in);
      }
      for (dof_id_type ss = 0; ss < (_sampler->getNumberOfRows()); ++ss)
      {
        for (dof_id_type i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
        {
          _inputs_sto[i].push_back(_data_in[(_sampler->parameters().get<std::vector<DistributionName>>("distributions").size())*ss+i]);
          _inputs[i][ss] = _data_in[(_sampler->parameters().get<std::vector<DistributionName>>("distributions").size())*ss+i];
        }
      }
      // std::cerr << "Here" << std::endl;
      // std::cerr << Moose::stringify((*_output)) << std::endl; // Moose::stringify((*_output))  (*_output)[0]
      _data = (_sampler->parameters().get<bool>("use_absolute_value")) ? AdaptiveMonteCarloUtils::computeABS((*_output)) : (*_output); // (*_output);
      // (*_output) = _data; // (_sampler->parameters().get<bool>("use_absolute_value")) ? AdaptiveMonteCarloUtils::computeABS((_output)) : (_output); //
      _communicator.allgather(_data);
      // std::cout << "Gather " << Moose::stringify(_data) << std::endl;
      for (dof_id_type ss = 0; ss < _data.size(); ++ss)
        _outputs_sto.push_back(_data[ss]);
      // std::cout << "sto " << Moose::stringify(_outputs_sto) << std::endl;
    } else
    {
      _subset = std::floor(((_step-1) * n_processors()) / _sampler->parameters().get<int>("num_samplessub"));
      _count_max = std::floor(1 / _sampler->parameters().get<Real>("subset_probability"));
      if (_subset > (std::floor(((_step-2) * n_processors()) /  _sampler->parameters().get<int>("num_samplessub"))))
      {
        _ind_sto = -1;
        _count = INT_MAX;
        _output_sorted = AdaptiveMonteCarloUtils::sortOUTPUT(_outputs_sto, _sampler->parameters().get<int>("num_samplessub"), _subset, _sampler->parameters().get<Real>("subset_probability"));
        for (dof_id_type j = 0; j < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++j)
        {
          _inputs_sorted[j].resize(std::floor(_sampler->parameters().get<int>("num_samplessub") * _sampler->parameters().get<Real>("subset_probability")));
          _inputs_sorted[j] = AdaptiveMonteCarloUtils::sortINPUT(_inputs_sto[j], _outputs_sto, _sampler->parameters().get<int>("num_samplessub"), _subset, _sampler->parameters().get<Real>("subset_probability"));
        }
        _output_limits.push_back(AdaptiveMonteCarloUtils::computeMIN(_output_sorted));
      }
      if (_count >= _count_max)
      {
        for (dof_id_type jj = 0; jj < n_processors(); ++jj)
        {
          ++_ind_sto;
          for (dof_id_type k = 0; k < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++k)
            _prev_val[k][jj] = _inputs_sorted[k][_ind_sto];
          _prev_val_out[jj] = _output_sorted[_ind_sto];
        }
        _count = 0;
      } else
      {
        for (dof_id_type jj = 0; jj < n_processors(); ++jj)
        {
          for (dof_id_type k = 0; k < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++k)
            _prev_val[k][jj] = _inputs_sto[k][_inputs_sto[k].size()-n_processors()+jj];
          _prev_val_out[jj] = _outputs_sto[_outputs_sto.size()-n_processors()+jj];
        }
      }
      ++_count;
      std::cout << "Reporter " << Moose::stringify(_prev_val) << std::endl;
      for (dof_id_type ss = _sampler->getLocalRowBegin(); ss < _sampler->getLocalRowEnd(); ++ss)
      {
        const auto data = _sampler->getNextLocalRow();
        _data_in = data;
        _communicator.allgather(_data_in);
      }
      _data = (_sampler->parameters().get<bool>("use_absolute_value")) ? AdaptiveMonteCarloUtils::computeABS((*_output)) :  (*_output); //;
      _communicator.allgather(_data);
      std::vector<Real> Tmp2 = _data;
      // std::cout << "Gather " << Moose::stringify(_data) << std::endl;
      for (dof_id_type ss = 0; ss < n_processors(); ++ss)
      {
        if (Tmp2[ss] >= _output_limits[_subset-1])
        {
          for (dof_id_type i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
          {
            _inputs[i][ss] = _data_in[(_sampler->parameters().get<std::vector<DistributionName>>("distributions").size())*ss+i];
            _inputs_sto[i].push_back(_inputs[i][ss]);
          }
          // (_output[ss]) = _data[ss];
          _outputs_sto.push_back(Tmp2[ss]);
        } else
        {
          for (dof_id_type i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
          {
            _inputs[i][ss] = _prev_val[i][ss];
            _data_in[(_sampler->parameters().get<std::vector<DistributionName>>("distributions").size())*ss+i] = _inputs[i][ss];
            _inputs_sto[i].push_back(_inputs[i][ss]);
          }
          Tmp2[ss] = _prev_val_out[ss];
          // (_output[ss]) = _prev_val_out[ss];
          _outputs_sto.push_back(Tmp2[ss]);
        }
        //
      }
      // std::cout << "sto " << Moose::stringify(_outputs_sto) << std::endl;
      _data = Tmp2;
      // std::cerr << Moose::stringify(_output_limits) << std::endl;
      // std::cerr << Moose::stringify(_outputs_sto) << std::endl;
    }




    // _subset = std::floor(_step / _sampler->parameters().get<int>("num_samplessub"));
    //
    // for (dof_id_type ss = _sampler->getLocalRowBegin(); ss < _sampler->getLocalRowEnd(); ++ss)
    // {
    //   const auto data = _sampler->getNextLocalRow();
    //   _data_in = data;
    //   _communicator.allgather(_data_in);
    // }
    //
    // for (dof_id_type ss = 0; ss < (_sampler->getNumberOfRows()); ++ss)
    // {
    //   for (dof_id_type i = 0; i < _sampler->parameters().get<std::vector<DistributionName>>("distributions").size(); ++i)
    //   {
    //     _inputs_sto[i].push_back(_data_in[(_sampler->parameters().get<std::vector<DistributionName>>("distributions").size())*ss+i]);
    //     _inputs[i][ss] = _data_in[(_sampler->parameters().get<std::vector<DistributionName>>("distributions").size())*ss+i];
    //   }
    // }
    //
    // _data = (*_output);
    // _communicator.allgather(_data);
    // for (dof_id_type ss = 0; ss < _data.size(); ++ss)
    //   _outputs_sto.push_back(_data[ss]);

  }
}
