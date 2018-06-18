//
// Created by kangd on 09.05.18.
//

#include <MjcSim.hpp>

#include "KaplaBenchmark.hpp"
#include "MjcBenchmark.hpp"

mujoco_sim::MjcSim *sim;
po::options_description desc;

void setupSimulation() {
  if (benchmark::building::options.gui)
    sim = new mujoco_sim::MjcSim(800, 600, 0.05,
                                      benchmark::building::getMujocoXMLpath().c_str(),
                                      benchmark::mujoco::getKeypath().c_str(),
                                      benchmark::NO_BACKGROUND,
                                      benchmark::mujoco::options.solverOption,
                                      benchmark::mujoco::options.integratorOption);
  else
    sim = new mujoco_sim::MjcSim(benchmark::building::getMujocoXMLpath().c_str(),
                                      benchmark::mujoco::getKeypath().c_str(),
                                      benchmark::mujoco::options.solverOption,
                                      benchmark::mujoco::options.integratorOption);

  // time step
  sim->setTimeStep(benchmark::building::options.dt);

  /// no erp for mujoco
  if(benchmark::building::options.erpYN)
    RAIFATAL("erp is not supported for mujoco")
}

void setupWorld() {

  if(benchmark::building::options.gui) {
    sim->setLightPosition((float)benchmark::building::params.lightPosition[0],
                          (float)benchmark::building::params.lightPosition[1],
                          (float)benchmark::building::params.lightPosition[2]);
    sim->cameraFollowObject(sim->getSingleBodyHandle(0), {0, 5, 2});
  }
}

benchmark::building::Data simulationLoop() {
  if(benchmark::building::options.saveVideo)
    sim->startRecordingVideo("/tmp", "mujoco-building");

  // data
  benchmark::building::Data data;
  data.setN(unsigned(benchmark::building::options.T / benchmark::building::options.dt));

  // timer start
  StopWatch watch;
  watch.start();

  int cnt = (int) (benchmark::building::options.T / benchmark::building::options.dt);
  for(int i = 0; i < (int) (benchmark::building::options.T / benchmark::building::options.dt); i++) {
    // gui
    if (benchmark::building::options.gui && !sim->visualizerLoop(benchmark::building::options.dt))
      break;

    // num contacts
    data.numContacts.push_back(sim->getWorldNumContacts());

    if(benchmark::building::options.collapse && sim->getSingleBodyHandle(sim->getNumObject()-1)->getPosition()[2] <
        benchmark::building::params.heightLen * (benchmark::building::params.numFloor - 1) * 2) {
      // break if the building collapses
      cnt = i+1;
      RAIINFO("building collapsed after " << cnt << " steps = " << cnt * benchmark::building::options.dt << " sec!")
      break;
    }

    sim->integrate();
  }

  if(benchmark::building::options.saveVideo)
    sim->stopRecordingVideo();

  data.time = watch.measure();
  data.step = cnt;
  return data;
}

int main(int argc, const char* argv[]) {

  benchmark::building::addDescToOption(desc);
  benchmark::mujoco::addDescToOption(desc);

  benchmark::building::getOptionsFromArg(argc, argv, desc);
  benchmark::mujoco::getOptionsFromArg(argc, argv, desc);

  benchmark::building::getParamsFromYAML(benchmark::building::getYamlpath().c_str(),
                                         benchmark::MUJOCO);

  setupSimulation();
  setupWorld();

  RAIINFO(
      std::endl << "=======================" << std::endl
                << "Simulator: " << benchmark::mujoco::options.simName << std::endl
                << "GUI      : " << benchmark::building::options.gui << std::endl
                << "ERP      : " << benchmark::building::options.erpYN << std::endl
                << "Timestep : " << benchmark::building::options.dt << std::endl
                << "Num block: " << sim->getNumObject()-1 << std::endl
                << "-----------------------"
  )

  benchmark::building::Data data = simulationLoop();

  if(benchmark::building::options.csv)
    benchmark::building::printCSV(benchmark::building::getCSVpath(),
                                  benchmark::mujoco::options.simName,
                                  benchmark::mujoco::options.solverName,
                                  benchmark::mujoco::options.detectorName,
                                  benchmark::mujoco::options.integratorName,
                                  data.time,
                                  data.step,
                                  data.computeMeanContacts());

  RAIINFO(
      std::endl << "Avg. Num Contacts : " << data.computeMeanContacts() << std::endl
                << "CPU time          : " << data.time << std::endl
                << "num steps         : " << data.step << std::endl
                << "=======================" << std::endl
  )


  delete sim;
  return 0;
}