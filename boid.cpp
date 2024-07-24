#include "boid.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/marker2d.hpp>
#include <godot_cpp/classes/collision_shape2d.hpp>
#include <godot_cpp/classes/circle_shape2d.hpp>
#include <godot_cpp/classes/sprite2d.hpp>

using namespace godot;

void Boid::_bind_methods()
{
	ClassDB::bind_method(D_METHOD("set_enable_boids", "toggle"), &Boid::set_enable_boids);
	ClassDB::bind_method(D_METHOD("get_enable_boids"), &Boid::get_enable_boids);
	ClassDB::bind_method(D_METHOD("set_separation_weight", "separation_weight"), &Boid::set_separation_weight);
	ClassDB::bind_method(D_METHOD("get_separation_weight"), &Boid::get_separation_weight);
	ClassDB::bind_method(D_METHOD("set_alignment_weight", "alignment_weight"), &Boid::set_alignment_weight);
	ClassDB::bind_method(D_METHOD("get_alignment_weight"), &Boid::get_alignment_weight);
	ClassDB::bind_method(D_METHOD("set_cohesion_weight", "cohesion_weight"), &Boid::set_cohesion_weight);
	ClassDB::bind_method(D_METHOD("get_cohesion_weight"), &Boid::get_cohesion_weight);
	ClassDB::bind_method(D_METHOD("set_collision_shape_radius", "collision_shape_radius"), &Boid::set_collision_shape_radius);
	ClassDB::bind_method(D_METHOD("get_collision_shape_radius"), &Boid::get_collision_shape_radius);
	ClassDB::bind_method(D_METHOD("set_nearby_boids"), &Boid::set_nearby_boids);
	ClassDB::bind_method(D_METHOD("get_nearby_boids"), &Boid::get_nearby_boids);
	ClassDB::bind_method(D_METHOD("ready"), &Boid::ready);
	ClassDB::bind_method(D_METHOD("update"), &Boid::update);
	ClassDB::bind_method(D_METHOD("apply_separation"), &Boid::apply_separation);
	ClassDB::bind_method(D_METHOD("_on_body_entered", "body"), &Boid::_on_body_entered);
	ClassDB::bind_method(D_METHOD("_on_body_exited", "body"), &Boid::_on_body_exited);

	ClassDB::add_property("Boid", PropertyInfo(Variant::BOOL, "enable_boids"), "set_enable_boids", "get_enable_boids");
	ClassDB::add_property("Boid", PropertyInfo(Variant::FLOAT, "separation_weight"), "set_separation_weight", "get_separation_weight");
	ClassDB::add_property("Boid", PropertyInfo(Variant::FLOAT, "alignment_weight"), "set_alignment_weight", "get_alignment_weight");
	ClassDB::add_property("Boid", PropertyInfo(Variant::FLOAT, "cohesion_weight"), "set_cohesion_weight", "get_cohesion_weight");
	ClassDB::add_property("Boid", PropertyInfo(Variant::FLOAT, "collision_shape_radius"), "set_collision_shape_radius", "get_collision_shape_radius");
	ClassDB::add_property("Boid", PropertyInfo(Variant::ARRAY, "nearby_boids"), "set_nearby_boids", "get_nearby_boids");
}

Boid::Boid()
{
	if (Engine::get_singleton()->is_editor_hint())
	{
		set_process_mode(Node::ProcessMode::PROCESS_MODE_DISABLED);
		set_process(false);
	}
}

Boid::~Boid() {}

void Boid::ready()
{
	try
	{
		boid_parent = Object::cast_to<CharacterBody2D>(get_parent());
		enemy_list = boid_parent->get("enemy_list");
		MAX_SPEED = boid_parent->get("MAX_SPEED");
		boid_center = Object::cast_to<Marker2D>(boid_parent->get_node<Marker2D>("Marker2D"));
		acceleration = boid_parent->get("ACCELERATION");
		deceleration = boid_parent->get("DECELERATION");
		set_process_mode(Node::ProcessMode::PROCESS_MODE_PAUSABLE);
		set_process(true);

		Sprite2D* boid_sprite = boid_parent->get_node<Sprite2D>("Sprite2D");

		CollisionShape2D* collisionShape = memnew(CollisionShape2D);
		add_child(collisionShape);
		collisionShape->set_position(Vector2(0, -12 - boid_sprite->get_texture()->get_height() * 0.5));

		CircleShape2D* circleShape = memnew(CircleShape2D);
		circleShape->set_radius(collision_shape_radius);
		collisionShape->set_shape(circleShape);

		connect("body_entered", callable_mp(this, &Boid::_on_body_entered));
		connect("body_exited", callable_mp(this, &Boid::_on_body_exited));
	}
	catch (const std::exception& e)
	{
		UtilityFunctions::print(static_cast<String>(e.what()));
	}
}

void Boid::_on_body_entered(Node2D* body)
{
	if (body != boid_parent)
		nearby_boids.push_back(body);
}

void Boid::_on_body_exited(Node2D* body)
{
	nearby_boids.erase(body);
}

void Boid::set_enable_boids(bool toggle)
{
	enable_boids = toggle;

	if (toggle)
	{
		set_process_mode(Node::ProcessMode::PROCESS_MODE_PAUSABLE);
		set_process(true);
	}
	else
	{
		set_process_mode(Node::ProcessMode::PROCESS_MODE_DISABLED);
		set_process(false);
	}
}

bool Boid::get_enable_boids() const
{
	return enable_boids;
}

void Boid::set_separation_weight(double weight)
{
	separation_weight = weight;
}

double Boid::get_separation_weight() const
{
	return separation_weight;
}

void Boid::set_alignment_weight(double weight)
{
	alignment_weight = weight;
}

double Boid::get_alignment_weight() const
{
	return alignment_weight;
}

void Boid::set_cohesion_weight(double weight)
{
	cohesion_weight = weight;
}

double Boid::get_cohesion_weight() const
{
	return cohesion_weight;
}

bool Boid::is_in_cone(Vector2 position, Vector2 direction, double angle_deg, double distance)
{
	Vector2 pos;

	try
	{
		if (boid_center == nullptr) return false;
		boid_center->get_global_position();
	}
	catch (const std::exception& e)
	{
		UtilityFunctions::print(static_cast<String>(e.what()));
	}

	Vector2 diff = (pos - position).normalized();
	double dot = diff.dot(direction);
	double ang_rad = angle_deg && Math::deg_to_rad(angle_deg) || 0.78539816339745;
	double cos = Math::cos(ang_rad);

	return dot > cos && pos.distance_squared_to(position) < distance * distance;
}

void Boid::_physics_process(double delta)
{
	if (!enable_boids) return;
	CharacterBody2D* enemy = boid_parent;
	if (enemy == nullptr) return;
	CharacterBody2D* player = Object::cast_to<CharacterBody2D>(enemy->get("player"));
	if (player == nullptr) return;

	Vector2 direction_to_player = enemy->get("global_position").call("direction_to", player->get("global_position"));
	Sprite2D* sprite = Object::cast_to<Sprite2D>(enemy->get("sprite"));

	if (sprite) {
		sprite->set_flip_h(direction_to_player.x < 0.0f);
	}

	if (direction_to_player.length_squared() < 1500.0f)
	{
		enemy->set_velocity(enemy->get_velocity().move_toward(direction_to_player * MAX_SPEED, acceleration));
	}
	else
	{
		enemy->set_velocity(enemy->get_velocity().move_toward(Vector2(), deceleration));
	}

	enemy->set_velocity(update(enemy->get_velocity()));
	enemy->move_and_collide(enemy->get_velocity() * delta, false, 0.055, true);
}

Vector2 Boid::update(Vector2 velocity)
{
	Vector2 separation;
	Vector2 alignment;
	Vector2 cohesion;

	for (int i = 0; i < nearby_boids.size(); i++)
	{
		CharacterBody2D* enemy = Object::cast_to<CharacterBody2D>(nearby_boids[i]);

		if (enemy == nullptr) continue;

		separation = calculate_separation(enemy, velocity);
		alignment = calculate_alignment(enemy, velocity);
		cohesion = calculate_cohesion(enemy);
	}

	Vector2 combined_force =
		(separation * separation_weight) +
		(alignment * alignment_weight) +
		(cohesion * cohesion_weight);

	Vector2 desired_velocity = velocity + combined_force;

	desired_velocity.limit_length(MAX_SPEED);

	return desired_velocity;
}

void Boid::set_collision_shape_radius(double r)
{
	collision_shape_radius = r;
}

double Boid::get_collision_shape_radius() const
{
	return collision_shape_radius;
}

void Boid::set_nearby_boids(Array boids)
{
	nearby_boids = boids;
}

Array Boid::get_nearby_boids() const
{
	return nearby_boids;
}

Vector2 Boid::calculate_separation(CharacterBody2D* enemy, Vector2 velocity)
{
	Vector2 force = Vector2();
	int count = 0;
	Vector2 enemy_center;

	enemy_center = Object::cast_to<Marker2D>(enemy->get_node<Marker2D>("Marker2D"))->get_global_position();

	Vector2 boid_center_position = boid_center->get_global_position();
	double distance = boid_center_position.distance_squared_to(enemy_center);
	Vector2 diff = enemy_center.direction_to(boid_center_position);

	if (distance > 0 && distance < separation_radius_sqr && is_in_cone(enemy_center, diff, 45, distance))
	{
		diff /= distance;
		force += diff;
		count++;
	}

	if (count > 0)
		force /= count;

	if (force.length_squared() > 0)
	{
		force = force.normalized();
		force = force.move_toward(-velocity.normalized(), 0.5);
		force *= MAX_SPEED;
		force.limit_length(MAX_SPEED);
	}

	return force;
}

Vector2 Boid::calculate_alignment(CharacterBody2D* enemy, Vector2 velocity)
{
	Vector2 force = Vector2();
	int count = 0;
	Vector2 enemy_center;

	enemy_center = Object::cast_to<Marker2D>(enemy->get_node<Marker2D>("Marker2D"))->get_global_position();

	double distance = boid_center->get_global_position().distance_squared_to(enemy_center);

	if (distance > 0 && distance < alignment_radius_sqr)
	{
		force += enemy->get_velocity();
		count++;
	}

	if (count > 0)
	{
		force /= count;
		force = force.normalized();
		force *= MAX_SPEED;
		Vector2 steer = force - velocity;
		steer.limit_length(MAX_SPEED);
		return steer;
	}

	return force;
}

Vector2 Boid::calculate_cohesion(CharacterBody2D* enemy)
{
	Vector2 force = Vector2();
	int count = 0;

	Vector2 boid_center_position = boid_center->get_global_position();
	Vector2 enemy_center;

	enemy_center = Object::cast_to<Marker2D>(enemy->get_node<Marker2D>("Marker2D"))->get_global_position();

	double distance = boid_center_position.distance_squared_to(enemy_center);

	if (distance > 0 && distance < cohesion_radius_sqr)
	{
		force += enemy_center;
		count++;
	}

	if (count > 0)
		force /= count;
		return force.direction_to(boid_center_position);

	return force;
}