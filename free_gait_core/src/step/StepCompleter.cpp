/*
 * StepCompleter.hpp
 *
 *  Created on: Mar 7, 2015
 *      Author: Péter Fankhauser
 *   Institute: ETH Zurich, Autonomous Systems Lab
 */

#include "free_gait_core/step/StepCompleter.hpp"
#include "free_gait_core/leg_motion/leg_motion.hpp"

namespace free_gait {

StepCompleter::StepCompleter(std::shared_ptr<free_gait::AdapterBase> adapter)
    : adapter_(adapter)
{
}

StepCompleter::~StepCompleter()
{
}

bool StepCompleter::complete(const State& state, Step& step) const
{
  for (auto& legMotion : step.legMotions_) {
    switch ((legMotion.second)->getType()) {
      case LegMotionBase::Type::Footstep:
        setParameters(dynamic_cast<Footstep&>(*legMotion.second));
        break;
      default:
        break;
    }
    switch (legMotion.second->getTrajectoryType()) {
      case LegMotionBase::TrajectoryType::EndEffector:
        if (!complete(state, step, dynamic_cast<EndEffectorMotionBase&>(*legMotion.second))) return false;
        break;
      case LegMotionBase::TrajectoryType::Joints:
//        if (!complete(state, step, *(legMotion.second))) return false;
        break;
      default:
        throw std::runtime_error("StepCompleter::complete() could not complete leg motion of this type.");
        break;
    }
  }

  if (step.baseMotion_) {
    switch (step.baseMotion_->getType()) {
      case BaseMotionBase::Type::Auto:
        setParameters(dynamic_cast<BaseAuto&>(*step.baseMotion_));
        break;
      default:
        break;
    }
    if (!complete(state, step, *(step.baseMotion_))) return false;
  }

  step.isComplete_ = true;
  return step.update();
}

bool StepCompleter::complete(const State& state, const Step& step, EndEffectorMotionBase& endEffectorMotion) const
{
  if (endEffectorMotion.getControlSetup().at(ControlLevel::Position)) {
    // TODO Check frame.
    endEffectorMotion.updateStartPosition(state.getPositionWorldToBaseInWorldFrame());
  }
//  if (baseMotion.getControlSetup().at(ControlLevel::Velocity)) {
//    // TODO
//  }
  endEffectorMotion.compute(state, step, *adapter_);
  return true;
}

bool StepCompleter::complete(const State& state, const Step& step, BaseMotionBase& baseMotion) const
{
  if (baseMotion.getControlSetup().at(ControlLevel::Position)) {
    // TODO Check frame.
    Pose pose(state.getPositionWorldToBaseInWorldFrame(), state.getOrientationWorldToBase());
    baseMotion.updateStartPose(pose);
  }
  if (baseMotion.getControlSetup().at(ControlLevel::Velocity)) {
    // TODO
  }
  baseMotion.compute(state, step, *adapter_);
  return true;
}

void StepCompleter::setParameters(Footstep& footstep) const
{
  if (footstep.surfaceNormal_ == Vector::Zero())
    footstep.surfaceNormal_ = footTargetParameters_.surfaceNormal;
  if (footstep.profileHeight_ == 0.0)
    footstep.profileHeight_ = footTargetParameters_.profileHeight;
  if (footstep.profileType_.empty())
    footstep.profileType_ = footTargetParameters_.profileType;
  if (footstep.averageVelocity_ == 0.0)
    footstep.averageVelocity_ = footTargetParameters_.averageVelocity;
}

void StepCompleter::setParameters(BaseAuto& baseAuto) const
{
  if (baseAuto.height_ == 0.0)
    baseAuto.height_ = baseAutoParameters_.height;
  if (baseAuto.averageLinearVelocity_ == 0.0)
    baseAuto.averageLinearVelocity_ = baseAutoParameters_.averageLinearVelocity;
  if (baseAuto.averageAngularVelocity_ == 0.0)
    baseAuto.averageAngularVelocity_ = baseAutoParameters_.averageAngularVelocity;
  if (baseAuto.supportMargin_ == 0.0)
    baseAuto.supportMargin_ = baseAutoParameters_.supportMargin;

  baseAuto.nominalPlanarStanceInBaseFrame_.clear();
  baseAuto.nominalPlanarStanceInBaseFrame_ = baseAutoParameters_.nominalPlanarStanceInBaseFrame;
}

} /* namespace */

