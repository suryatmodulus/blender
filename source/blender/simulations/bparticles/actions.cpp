#include "actions.hpp"

namespace BParticles {

class KillAction : public Action {
  void execute(ActionInterface &interface) override
  {
    ParticleSet &particles = interface.particles();

    auto kill_states = particles.attributes().get_byte("Kill State");
    for (uint pindex : particles.indices()) {
      kill_states[pindex] = 1;
    }
  }
};

class MoveAction : public Action {
 private:
  float3 m_offset;

 public:
  MoveAction(float3 offset) : m_offset(offset)
  {
  }

  void execute(ActionInterface &interface) override
  {
    ParticleSet &particles = interface.particles();

    auto positions = particles.attributes().get_float3("Position");
    for (uint pindex : particles.indices()) {
      positions[pindex] += m_offset;
    }
  }
};

class SpawnAction : public Action {
  void execute(ActionInterface &interface) override
  {
    ParticleSet &particles = interface.particles();

    auto positions = particles.attributes().get_float3("Position");

    SmallVector<float3> new_positions;
    SmallVector<float3> new_velocities;
    SmallVector<float> new_birth_times;

    for (uint i : particles.range()) {
      uint pindex = particles.get_particle_index(i);
      new_positions.append(positions[pindex] + float3(20, 0, 0));
      new_velocities.append(float3(1, 1, 10));
      new_birth_times.append(interface.current_times()[i]);
    }

    auto &target = interface.request_emit_target(0, particles.size());
    target.set_float3("Position", new_positions);
    target.set_float3("Velocity", new_velocities);
    target.set_float("Birth Time", new_birth_times);
  }
};

static float3 random_direction()
{
  return float3(
      (rand() % 1000 - 500) / 500.f, (rand() % 1000 - 500) / 500.f, (rand() % 1000 - 500) / 500.f);
}

class ExplodeAction : public Action {
  void execute(ActionInterface &interface) override
  {
    ParticleSet &particles = interface.particles();

    auto positions = particles.attributes().get_float3("Position");
    auto kill_states = particles.attributes().get_byte("Kill State");
    auto current_times = interface.current_times();

    SmallVector<float3> new_positions;
    SmallVector<float3> new_velocities;
    SmallVector<float> new_birth_times;

    uint parts_amount = 100;

    for (uint i : particles.range()) {
      uint pindex = particles.get_particle_index(i);

      kill_states[pindex] = 1;
      new_positions.append_n_times(positions[pindex], parts_amount);
      new_birth_times.append_n_times(current_times[i], parts_amount);
      for (uint j = 0; j < parts_amount; j++) {
        new_velocities.append(random_direction() * 4);
      }
    }

    auto &target = interface.request_emit_target(1, new_positions.size());
    target.set_float3("Position", new_positions);
    target.set_float3("Velocity", new_velocities);
    target.set_float("Birth Time", new_birth_times);
  }
};

std::unique_ptr<Action> ACTION_kill()
{
  Action *action = new KillAction();
  return std::unique_ptr<Action>(action);
}

std::unique_ptr<Action> ACTION_move(float3 offset)
{
  Action *action = new MoveAction(offset);
  return std::unique_ptr<Action>(action);
}

std::unique_ptr<Action> ACTION_spawn()
{
  Action *action = new SpawnAction();
  return std::unique_ptr<Action>(action);
}

std::unique_ptr<Action> ACTION_explode()
{
  Action *action = new ExplodeAction();
  return std::unique_ptr<Action>(action);
}

}  // namespace BParticles