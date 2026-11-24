#include "game/ai/bot_input.hpp"
#include "game/physics/collision.hpp"
#include "game/world/world.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace tanks {
namespace {
constexpr int gridWidth = 40, gridCount = gridWidth * gridWidth;
constexpr float engagementDistance = 7.0f;
int gridIndex(int x, int z) { return z * gridWidth + x; }
Vec2 gridPoint(int i, float radius) {
	const float step = 2.0f * radius / static_cast<float>(gridWidth - 1);
	const int column = i % gridWidth;
	const int row = i / gridWidth;
	return {-radius + column * step, -radius + row * step};
}
void markBlocked(const BoxCollider &collider, const Vec2 &point,
                 const float &radius, bool &blocked) {
	if (!blocked && detail::circleBox(point, radius, collider))
		blocked = true;
}
int nearestCell(const std::array<bool, gridCount> &blocked, float radius,
                Vec2 point) {
	int result = -1;
	float nearest = std::numeric_limits<float>::infinity();
	for (int i = 0; i < gridCount; ++i)
		if (!blocked[i]) {
			const Vec2 delta = gridPoint(i, radius) - point;
			const float distance = dot(delta, delta);
			if (distance < nearest) {
				nearest = distance;
				result = i;
			}
		}
	return result;
}
} // namespace

void BotInput::planPath(Vec2 goal, World &world) {
	Navigation &bot = bot_;
	bot.pathSize = bot.waypoint = 0;
	const float radius = world.config().rules.arenaHalfExtent;
	const float tankRadius = tank_.collider.radius;
	std::array<bool, gridCount> blocked{};
	for (int i = 0; i < gridCount; ++i) {
		const Vec2 p = gridPoint(i, radius);
		blocked[i] = !world.arena().contains(p, tankRadius + 0.05f);
		world.forEachBoxCollider(markBlocked, p, tankRadius + 0.15f,
		                         blocked[i]);
	}
	const int start = nearestCell(blocked, radius, tank_.transform().position),
	          target = nearestCell(blocked, radius, goal);
	if (start < 0 || target < 0)
		return;
	std::array<float, gridCount> cost;
	cost.fill(std::numeric_limits<float>::infinity());
	std::array<int, gridCount> parent;
	parent.fill(-1);
	std::array<bool, gridCount> closed{};
	cost[start] = 0;
	const int targetX = target % gridWidth, targetZ = target / gridWidth;
	for (int iteration = 0; iteration < gridCount; ++iteration) {
		int current = -1;
		float best = std::numeric_limits<float>::infinity();
		for (int i = 0; i < gridCount; ++i)
			if (!closed[i] && std::isfinite(cost[i])) {
				const float estimate = cost[i] + float(std::abs(i % gridWidth - targetX) +
				                                       std::abs(i / gridWidth - targetZ));
				if (estimate < best) {
					current = i;
					best = estimate;
				}
			}
		if (current < 0 || current == target)
			break;
		closed[current] = true;
		const int x = current % gridWidth, z = current / gridWidth;
		const int dx[4] = {1, -1, 0, 0}, dz[4] = {0, 0, 1, -1};
		for (int k = 0; k < 4; ++k) {
			const int nx = x + dx[k], nz = z + dz[k];
			if (nx < 0 || nx >= gridWidth || nz < 0 || nz >= gridWidth)
				continue;
			const int next = gridIndex(nx, nz);
			if (blocked[next] || closed[next])
				continue;
			const float candidate = cost[current] + 1;
			if (candidate < cost[next]) {
				cost[next] = candidate;
				parent[next] = current;
			}
		}
	}
	if (target != start && parent[target] < 0)
		return;
	std::array<int, gridCount> reverse{};
	std::size_t count = 0;
	for (int cursor = target;
	     cursor != start && cursor >= 0 && count < reverse.size();
	     cursor = parent[cursor])
		reverse[count++] = cursor;
	bot.path[bot.pathSize++] = gridPoint(start, radius);
	while (count && bot.pathSize < bot.path.size())
		bot.path[bot.pathSize++] = gridPoint(reverse[--count], radius);
}

void BotInput::updateControls(const PlayerInputs &, float dt, World &world) {
	Command cmd;
	TankId target = noTank;
	float best = std::numeric_limits<float>::infinity();
	for (const Tank &candidate : world.tanks()) {
		if (!candidate.active() || candidate.control.team == tank_.control.team)
			continue;
		const Vec2 delta =
		    candidate.transform().position - tank_.transform().position;
		const float distanceSquared = dot(delta, delta);
		if (distanceSquared < best) {
			best = distanceSquared;
			target = candidate.id;
		}
	}
	if (target == noTank) {
		bot_.target = noTank;
		tank_.apply(cmd, dt, world);
		return;
	}
	Navigation &bot = bot_;
	bot.replan -= dt;
	bot.fireDelay -= dt;
	const bool changedTarget = bot.target != target;
	bot.target = target;
	const Tank &enemy = world.tanks()[target];
	const Vec2 aim = enemy.transform().position;
	const Vec2 toTarget = aim - tank_.transform().position;
	const float distance = length(toTarget);
	const bool inSight = world.lineOfSight(tank_.transform().position, aim);
	if (changedTarget)
		bot.fireDelay = std::max(bot.fireDelay, 0.6f);

	cmd.aim = aim;
	cmd.hasAim = true;
	if (distance > engagementDistance || !inSight) {
		if (changedTarget || bot.replan <= 0.0f || bot.waypoint >= bot.pathSize) {
			bot.replan = 0.8f + world.randomUnit() * 0.45f;
			planPath(enemy.transform().position, world);
		}
		while (bot.waypoint + 1 < bot.pathSize &&
		       length(bot.path[bot.waypoint] - tank_.transform().position) < 0.3f)
			++bot.waypoint;
		const Vec2 goal = bot.waypoint < bot.pathSize ? bot.path[bot.waypoint]
		                                              : enemy.transform().position;
		const Vec2 move = normalized(goal - tank_.transform().position);
		Vec2 repel{};
		for (const Tank &ally : world.tanks())
			if (ally.active() && ally.id != tank_.id &&
			    ally.control.team == tank_.control.team) {
				const Vec2 away = tank_.transform().position - ally.transform().position;
				const float allyDistance = length(away);
				if (allyDistance > detail::epsilon &&
				    allyDistance < tank_.collider.radius * 3.0f)
					repel += normalized(away) * (1.0f / allyDistance);
			}
		cmd.move = normalized(move + repel * 1.8f);
	} else if (distance < engagementDistance) {
		const Vec2 strafe = normalized({-toTarget.z, toTarget.x});
		cmd.move = strafe * 0.55f - normalized(toTarget) * 0.45f;
	}
	const float desired = angleOf(aim - tank_.transform().position),
	            misalignment =
	                std::abs(wrapAngle(desired - tank_.weapon.turretAngle));
	cmd.fire = inSight && bot.fireDelay <= 0.0f && misalignment < 0.10f;
	if (cmd.fire)
		bot.fireDelay = 1.4f + world.randomUnit() * 0.4f;
	tank_.apply(cmd, dt, world);
}

void BotInput::resetControls() { bot_ = {}; }
} // namespace tanks
