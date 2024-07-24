#pragma once

#include <godot_cpp/classes/area2d.hpp>
#include <godot_cpp/classes/character_body2d.hpp>
#include <godot_cpp/classes/marker2d.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/classes/engine.hpp>

namespace godot
{
	class Boid : public Area2D
	{
		GDCLASS(Boid, Area2D);

		public:
			Boid();
			~Boid();
			void set_enable_boids(bool toggle);
			bool get_enable_boids() const;
			void set_separation_weight(double weight);
			double get_separation_weight() const;
			void set_alignment_weight(double weight);
			double get_alignment_weight() const;
			void set_cohesion_weight(double weight);
			double get_cohesion_weight() const;
			void set_collision_shape_radius(double collision_shape_radius);
			double get_collision_shape_radius() const;
			void set_nearby_boids(Array boids);
			void _on_body_entered(Node2D* body);
			void _on_body_exited(Node2D* body);
			Array get_nearby_boids() const;
			bool enable_boids = true;
			double separation_weight = 1.5f;
			double alignment_weight = 1.0f;
			double cohesion_weight = 1.0f;
			double collision_shape_radius = 12.0f;
			double separation_radius = collision_shape_radius;
			double alignment_radius = collision_shape_radius;
			double cohesion_radius = collision_shape_radius;
			double separation_radius_sqr = separation_radius * separation_radius;
			double alignment_radius_sqr = alignment_radius * alignment_radius;
			double cohesion_radius_sqr = cohesion_radius * cohesion_radius;
			Array nearby_boids;
			CharacterBody2D* boid_parent;
			real_t acceleration = 0.0f;
			real_t deceleration = 0.0f;
			Marker2D* boid_center;
			void ready();
			virtual void _physics_process(double delta) override;
			Vector2 update(Vector2 velocity);
			double MAX_SPEED;
			Object* global;
			Array enemy_list = Array();

		protected:
			static void _bind_methods();

		private:
			bool is_in_cone(Vector2 position, Vector2 direction, double angle_deg, double distance);
			Vector2 calculate_separation(CharacterBody2D* enemy, Vector2 velocity);
			Vector2 calculate_alignment(CharacterBody2D* enemy, Vector2 velocity);
			Vector2 calculate_cohesion(CharacterBody2D* enemy);
			Vector2 apply_separation();
	};
}