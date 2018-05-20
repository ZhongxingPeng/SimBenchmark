//
// Created by kangd on 10.02.18.
//

#include "BtWorld.hpp"

namespace bullet_sim {

BtWorld::BtWorld(SolverOption solverOption) : solverOption_(solverOption) {

  // broadphase
  broadphase_ = new btDbvtBroadphase();

  // collision configuration and dispatcher
  collisionConfiguration_ = new btDefaultCollisionConfiguration();
  collisionConfiguration_->setConvexConvexMultipointIterations();
  collisionConfiguration_->setPlaneConvexMultipointIterations(5);
  collisionDispatcher_ = new btCollisionDispatcher(collisionConfiguration_);

  // physics solver
  switch(solverOption) {
    case SOLVER_SEQUENTIAL_IMPULSE:
      solver_ = new btSequentialImpulseConstraintSolver;
      break;
    case SOLVER_NNCG:
      solver_ = new btNNCGConstraintSolver;
      break;
    case SOLVER_MLCP_PGS:
      mlcpSolver_ = new btSolveProjectedGaussSeidel;
      solver_ = new btMLCPSolver(mlcpSolver_);
      break;
    case SOLVER_MLCP_DANTZIG:
      mlcpSolver_ = new btDantzigSolver;
      solver_ = new btMLCPSolver(mlcpSolver_);
      break;
    case SOLVER_MLCP_LEMKE:
      mlcpSolver_ = new btLemkeSolver;
      solver_ = new btMLCPSolver(mlcpSolver_);
      break;
    case SOLVER_MULTI_BODY:
      solver_ = new btMultiBodyConstraintSolver;
      break;
    default:
      solver_ = new btSequentialImpulseConstraintSolver;
      break;
  }

  // world
  if(solverOption == SOLVER_MULTI_BODY) {
    // multi body world
    dynamicsWorld_ = new btMultiBodyDynamicsWorld(collisionDispatcher_,
                                                  broadphase_,
                                                  (btMultiBodyConstraintSolver* )solver_,
                                                  collisionConfiguration_);
  }
  else {
    // single body world
    dynamicsWorld_ = new btDiscreteDynamicsWorld(collisionDispatcher_,
                                                 broadphase_,
                                                 solver_,
                                                 collisionConfiguration_);
  }

  dynamicsWorld_->setGravity(gravity_);

  // solver properties
//  dynamicsWorld_->getSolverInfo().m_tau = btScalar(0.6);
//  dynamicsWorld_->getSolverInfo().m_damping = btScalar(1.0);
//  dynamicsWorld_->getSolverInfo().m_friction = btScalar(0.8);
//  dynamicsWorld_->getSolverInfo().m_restitution = 0.0;
//  dynamicsWorld_->getSolverInfo().m_maxErrorReduction = btScalar(20.);
//  dynamicsWorld_->getSolverInfo().m_numIterations = 1000;                     // TODO
  dynamicsWorld_->getSolverInfo().m_erp = 0;
  dynamicsWorld_->getSolverInfo().m_erp2 = 0;
  dynamicsWorld_->getSolverInfo().m_frictionERP = 0;
//  dynamicsWorld_->getSolverInfo().m_globalCfm = btScalar(0.0);
//  dynamicsWorld_->getSolverInfo().m_frictionCFM = 0;
//  dynamicsWorld_->getSolverInfo().m_sor = btScalar(1.);
//  dynamicsWorld_->getSolverInfo().m_splitImpulse = false;
//  dynamicsWorld_->getSolverInfo().m_splitImpulsePenetrationThreshold = 0;
//  dynamicsWorld_->getSolverInfo().m_splitImpulseTurnErp = 0;
//  dynamicsWorld_->getSolverInfo().m_linearSlop = 0;
  dynamicsWorld_->getSolverInfo().m_warmstartingFactor= 0;
//  dynamicsWorld_->getSolverInfo().m_solverMode = SOLVER_SIMD;// | SOLVER_RANDMIZE_ORDER;
//  dynamicsWorld_->getSolverInfo().m_restingContactRestitutionThreshold = 2;//unused as of 2.81
//  dynamicsWorld_->getSolverInfo().m_minimumSolverBatchSize = 128; //try to combine islands until the amount of constraints reaches this limit
//  dynamicsWorld_->getSolverInfo().m_singleAxisRollingFrictionThreshold = 1e30f;///if the velocity is above this threshold, it will use a single constraint row (axis), otherwise 3 rows.
//  dynamicsWorld_->getSolverInfo().m_leastSquaresResidualThreshold = 0.f;
  dynamicsWorld_->getSolverInfo().m_restitutionVelocityThreshold = 0.0f;//if the relative velocity is below this threshold, there is zero restitution

}

BtWorld::~BtWorld() {

  // remove world
  delete dynamicsWorld_;

  // remove solver
  if(mlcpSolver_)
    delete mlcpSolver_;
  delete solver_;

  delete collisionDispatcher_;
  delete collisionConfiguration_;
  delete broadphase_;

  // remove objects
  for (auto *ob: objectList_)
    delete ob;
}

bullet_sim::object::BtSphere *bullet_sim::BtWorld::addSphere(double radius,
                                                         double mass,
                                                         benchmark::CollisionGroupType collisionGroup,
                                                         benchmark::CollisionGroupType collisionMask) {
  auto *sphere = new bullet_sim::object::BtSphere(radius, mass);
  dynamicsWorld_->addRigidBody(sphere->getRigidBody(), collisionGroup, collisionMask);
  objectList_.push_back(sphere);
  return sphere;
}

bullet_sim::object::BtBox *bullet_sim::BtWorld::addBox(double xLength,
                                                   double yLength,
                                                   double zLength,
                                                   double mass,
                                                   benchmark::CollisionGroupType collisionGroup,
                                                   benchmark::CollisionGroupType collisionMask) {
  auto *box = new bullet_sim::object::BtBox(xLength, yLength, zLength, mass);
  dynamicsWorld_->addRigidBody(box->getRigidBody(), collisionGroup, collisionMask);
  objectList_.push_back(box);
  return box;
}

bullet_sim::object::BtCapsule *BtWorld::addCapsule(double radius,
                                               double height,
                                               double mass,
                                               benchmark::CollisionGroupType collisionGroup,
                                               benchmark::CollisionGroupType collisionMask) {
  auto *capsule = new bullet_sim::object::BtCapsule(radius, height, mass);
  dynamicsWorld_->addRigidBody(capsule->getRigidBody(), collisionGroup, collisionMask);
  objectList_.push_back(capsule);
  return capsule;
}

bullet_sim::object::BtCheckerBoard *bullet_sim::BtWorld::addCheckerboard(double gridSize,
                                                                     double xLength,
                                                                     double yLength,
                                                                     double reflectanceI,
                                                                     bo::CheckerboardShape shape,
                                                                     benchmark::CollisionGroupType collisionGroup,
                                                                     benchmark::CollisionGroupType collisionMask) {
  auto *checkerBoard = new bullet_sim::object::BtCheckerBoard(xLength, yLength, shape);
  dynamicsWorld_->addRigidBody(checkerBoard->getRigidBody(), collisionGroup, collisionMask);
  objectList_.push_back(checkerBoard);
  return checkerBoard;
}

bullet_sim::object::BtCylinder *bullet_sim::BtWorld::addCylinder(double radius,
                                                             double height,
                                                             double mass,
                                                             benchmark::CollisionGroupType collisionGroup,
                                                             benchmark::CollisionGroupType collisionMask) {
  auto *cylinder = new bullet_sim::object::BtCylinder(radius, height, mass);
  dynamicsWorld_->addRigidBody(cylinder->getRigidBody(), collisionGroup, collisionMask);
  objectList_.push_back(cylinder);
  return cylinder;
}

object::BtArticulatedSystem *BtWorld::addArticulatedSystem(std::string urdfPath,
                                                       benchmark::CollisionGroupType collisionGroup,
                                                       benchmark::CollisionGroupType collisionMask) {
  object::BtArticulatedSystem *articulatedSystem = new bullet_sim::object::BtArticulatedSystem(urdfPath,
                                                                                               (btMultiBodyDynamicsWorld* )dynamicsWorld_);
  objectList_.push_back(articulatedSystem);
  return articulatedSystem;
}

void bullet_sim::BtWorld::integrate(double dt) {
  // simulation step
  dynamicsWorld_->stepSimulation(dt, 0);

  // clear collision
  contactProblemList_.clear();
  int contactProblemSize = 0;
  for (int i = 0; i < collisionDispatcher_->getNumManifolds(); i++)
    contactProblemSize += collisionDispatcher_->getManifoldByIndexInternal(i)->getNumContacts();
  contactProblemList_.reserve(contactProblemSize);

  for (int i = 0; i < collisionDispatcher_->getNumManifolds(); i++)
  {
    btPersistentManifold* contactManifold = collisionDispatcher_->getManifoldByIndexInternal(i);
//    const btCollisionObject* obA = contactManifold->getBody0();
//    const btCollisionObject* obB = contactManifold->getBody1();

    for (int j = 0; j < contactManifold->getNumContacts(); j++)
    {
      btManifoldPoint& pt = contactManifold->getContactPoint(j);
      if (pt.getDistance() <= 0.f)
      {
        const btVector3& ptA = pt.getPositionWorldOnA();
//        const btVector3& ptB = pt.getPositionWorldOnB();
        const btVector3& normalOnB = pt.m_normalWorldOnB;
        contactProblemList_.emplace_back(ptA, normalOnB);
      }
    }
  }
}

const std::vector<Single3DContactProblem> *BtWorld::getCollisionProblem() const {
  return &contactProblemList_;
}

void bullet_sim::BtWorld::setGravity(const benchmark::Vec<3> &gravity) {
  gravity_ = {gravity[0],
              gravity[1],
              gravity[2]};
  dynamicsWorld_->setGravity(gravity_);
}

void BtWorld::setERP(double erp, double erp2, double frictionErp) {
  dynamicsWorld_->getSolverInfo().m_erp = erp;
  dynamicsWorld_->getSolverInfo().m_erp2 = erp2;
  dynamicsWorld_->getSolverInfo().m_frictionERP = frictionErp;
}

int BtWorld::getNumObject() {
  objectList_.size();
}

void BtWorld::setMultipointIteration(int convexconvex, int convexplane) {
  collisionConfiguration_->setConvexConvexMultipointIterations(convexconvex);
  collisionConfiguration_->setPlaneConvexMultipointIterations(convexplane);
}

void BtWorld::integrate1(double dt) {
  RAIFATAL("not supported for bullet")
}
void BtWorld::integrate2(double dt) {
  RAIFATAL("not supported for bullet")
}

} // bullet_sim
