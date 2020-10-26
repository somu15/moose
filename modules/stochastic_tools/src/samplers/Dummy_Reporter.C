//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Dummy_Reporter.h"
#include "Distribution.h"
#include "Normal.h"

registerMooseObjectAliased("StochasticToolsApp", Dummy_Reporter, "Dummy_Reporter");
registerMooseObjectReplaced("StochasticToolsApp",
                            Dummy_Reporter,
                            "07/01/2020 00:00",
                            MonteCarlo);
registerMooseObject("StochasticToolsApp", TestDeclareReporter);
// registerMooseObject("MooseTestApp", TestGetReporter);

InputParameters
Dummy_Reporter::validParams()
{
  InputParameters params = Sampler::validParams();
  params.addClassDescription("Monte Carlo Sampler.");
  params.addRequiredParam<dof_id_type>("num_rows", "The number of rows per matrix to generate.");
  params.addRequiredParam<std::vector<DistributionName>>(
      "distributions",
      "The distribution names to be sampled, the number of distributions provided defines the "
      "number of columns per matrix.");
  params.addParam<VectorPostprocessorName>(
      "results_vpp", "Vectorpostprocessor with results of samples created by trainer.");
  params.addRequiredParam<VectorPostprocessorName>(
      "inputs_vpp", "Vectorpostprocessor with results of samples created by trainer.");
  params.addParam<std::vector<std::string>>(
      "vpp_names",
      "Name of vector from vectorpostprocessor with results of samples created by trainer");
  params.addRequiredParam<std::vector<std::string>>(
      "inputs_names",
      "Name of vector from vectorpostprocessor with inputs of samples created by trainer");
  params.addRequiredParam<std::vector<Real>>(
      "proposal_std",
      "Standard deviations of the proposal distributions");
  params.addParam<std::vector<Real>>(
      "vpp_limits",
      "Limit values of the VPPs");
  params.addRequiredParam<std::vector<Real>>(
      "seeds",
      "Seed input values to get the MCMC sampler started");
  params.addParam<Real>(
      "num_samples_adaptive",
      "Number of samples after which adaptive MCMC is activated");
  return params;
}

Dummy_Reporter::Dummy_Reporter(const InputParameters & parameters)
  : Sampler(parameters), VectorPostprocessorInterface(this),
    _distribution_names(getParam<std::vector<DistributionName>>("distributions")),
    _vpp_names(getParam<std::vector<std::string>>("vpp_names")),
    _inputs_names(getParam<std::vector<std::string>>("inputs_names")),
    _proposal_std(getParam<std::vector<Real>>("proposal_std")),
    _vpp_limits(getParam<std::vector<Real>>("vpp_limits")),
    _seeds(getParam<std::vector<Real>>("seeds")),
    _num_samples_adaptive(getParam<Real>("num_samples_adaptive")),
    _step(getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")->timeStep()),
    _perf_compute_sample(registerTimedSection("computeSample", 4))
{
  for (const DistributionName & name : _distribution_names)
    _distributions.push_back(&getDistributionByName(name));

  setNumberOfRows(getParam<dof_id_type>("num_rows"));
  setNumberOfCols(_distributions.size());
  _new_sample_vec.resize(_distributions.size());
  _new_sample_sca = 0.0;
  _acceptance_ratio = 0.0;
  _consecutive_indicator = 0;
  _sum_inputs.resize(_distributions.size() * getParam<dof_id_type>("num_rows"));
  _diff_inputs.resize(_distributions.size() * getParam<dof_id_type>("num_rows"));
  _store_sample.resize(_distributions.size() * getParam<dof_id_type>("num_rows"));
}

void
Dummy_Reporter::sampleSetUp()
{
  if (isParamValid("results_vpp"))
  {
    _values_distributed = isVectorPostprocessorDistributed("results_vpp");
    _values_ptr.resize(_vpp_names.size() + 1);
    for (unsigned int i = 0; i < _vpp_names.size(); ++i)
    {
      _values_ptr[i] = getVectorPostprocessorValue(
          "results_vpp", _vpp_names[i], !_values_distributed);
    }
  }

  _values_distributed1 = isVectorPostprocessorDistributed("inputs_vpp");
  _inputs_ptr.resize(_inputs_names.size() + 1);
  for (unsigned int i = 0; i < _inputs_names.size(); ++i)
  {
    _inputs_ptr[i] = getVectorPostprocessorValue(
        "inputs_vpp", _inputs_names[i], !_values_distributed1);
  }
}

Real
Dummy_Reporter::computeSample(dof_id_type row_index, dof_id_type col_index)
{
  TIME_SECTION(_perf_compute_sample);
  dof_id_type offset = _values_distributed ? getLocalRowBegin() : 0;
  if (_step > 1)
  {
    if (isParamValid("results_vpp"))
    {
      return _seeds[col_index];
    } else
    {
      // std::cout << getGlobalSamples() << std::endl;
      _new_sample_sca = Normal::quantile(getRand(), _inputs_ptr[col_index][row_index-offset], _proposal_std[col_index]);
      _acceptance_ratio = _distributions[col_index]->pdf(_new_sample_sca) / _distributions[col_index]->pdf(_inputs_ptr[col_index][row_index-offset]);
      if (_acceptance_ratio > getRand())
      {
        return _new_sample_sca;
      } else
      {
        return _inputs_ptr[col_index][row_index-offset];
      }
    }
  } else
  {
    return _seeds[col_index];
  }
}

InputParameters
TestDeclareReporter::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addParam<ReporterValueName>(
      "int_name", "int", "The name of the interger data"); // MooseDocs:data
  return params;
}

TestDeclareReporter::TestDeclareReporter(const InputParameters & parameters)
  : GeneralReporter(parameters),
    // _int(declareValue<int>("int_name", 1980)), // MooseDocs:producer
    _real(declareValueByName<Real>("real"))
    // _vector(declareValueByName<std::vector<Real>>("vector")),
    // _string(declareValueByName<std::string>("string")),
    // _bcast_value(declareValueByName<Real, ReporterBroadcastContext>("broadcast")),
    // _scatter_value(
    //     declareValueByName<dof_id_type, ReporterScatterContext>("scatter", _values_to_scatter)),
    // _gather_value(declareValueByName<std::vector<dof_id_type>, ReporterGatherContext>(
    //     "gather")) // MooseDocs:gather
{
  // if (processor_id() == 0)
  //   for (dof_id_type rank = 0; rank < n_processors(); ++rank)
  //     _values_to_scatter.push_back(rank);
}

// MooseDocs:execute_begin
void
TestDeclareReporter::execute()
{
  // _int += 1;
  _real = 2.0;
  std::cout << _real << std::endl;
  // _vector = {1, 1.1, 1.2};
  // _string = "string";
  //
  // if (processor_id() == 0)
  //   _bcast_value = 42;
  //
  // _gather_value.resize(1, processor_id());
}
// MooseDocs:execute_end
