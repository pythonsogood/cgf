#include "graphics/ui/hud.hpp"
#include "game/world/world.hpp"
#include "imgui.h"
#include "backends/imgui_impl_opengl3.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <stdexcept>

namespace tanks::graphics {
namespace {
constexpr ImGuiWindowFlags anchored =
    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
    ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;
constexpr ImGuiWindowFlags passive = anchored | ImGuiWindowFlags_NoDecoration |
                                     ImGuiWindowFlags_NoInputs |
                                     ImGuiWindowFlags_NoFocusOnAppearing;

void drawPlayerCards(const World &world, float width, float height) {
	const ImGuiStyle &style = ImGui::GetStyle();
	const auto &tanks = world.tanks();

	const float gap = style.ItemSpacing.x;
	const int count = static_cast<int>(tanks.size());
	const int capacity =
	    std::max(1, static_cast<int>((width - gap) / (112.0f + gap)));

	const int rows = (count + capacity - 1) / capacity;
	const int columns = (count + rows - 1) / rows;

	const float cardWidth =
	    std::clamp((width - (columns + 1) * gap) / columns, 1.0f, 210.0f);
	const float cardHeight = 2 * style.WindowPadding.y +
	                         ImGui::GetTextLineHeightWithSpacing() +
	                         ImGui::GetFrameHeight();

	for (const Tank &tank : tanks) {
		const int row = int(tank.id) / columns, col = int(tank.id) % columns;
		const int inRow = std::min(columns, count - row * columns);

		const float left = (width - inRow * cardWidth - (inRow - 1) * gap) * .5f;
		const float bottom = height - gap - (rows - row - 1) * (cardHeight + gap);

		ImGui::SetNextWindowPos({left + col * (cardWidth + gap), bottom},
		                        ImGuiCond_Always, {0, 1});
		ImGui::SetNextWindowSize({cardWidth, cardHeight});

		char id[32];

		std::snprintf(id, sizeof(id), "##tank_%zu", tank.id);

		if (ImGui::Begin(id, nullptr, passive)) {
			const auto &team = world.config().teams[tank.control.team];
			const Color c = team.color;

			ImGui::TextColored({c.r, c.g, c.b, 1}, "%s", team.name.c_str());
			ImGui::SameLine();
			ImGui::TextUnformatted(tank.control.controller == Controller::Human ? "YOU"
			                                                                    : "BOT");
			char hp[32], wins[32];
			std::snprintf(hp, sizeof(hp), "%.0f HP", tank.health.current);
			std::snprintf(wins, sizeof(wins), "%u W",
			              world.teams()[tank.control.team].wins);

			const float barWidth =
			    ImGui::GetContentRegionAvail().x - gap - ImGui::CalcTextSize(wins).x;

			ImGui::ProgressBar(tank.health.current / tank.health.maximum,
			                   {std::max(1.0f, barWidth), 0}, hp);
			ImGui::SameLine();
			ImGui::TextUnformatted(wins);
		}

		ImGui::End();
	}
}
} // namespace

HudRenderer::HudRenderer() {
	IMGUI_CHECKVERSION();
	context_ = ImGui::CreateContext();
	if (!context_)
		throw std::runtime_error("Dear ImGui context creation failed");

	ImGui::SetCurrentContext(context_);
	try {
		ImGuiIO &io = ImGui::GetIO();
		io.IniFilename = nullptr;
		io.LogFilename = nullptr;
		ImGui::StyleColorsDark();
		if (!ImGui_ImplOpenGL3_Init("#version 330 core"))
			throw std::runtime_error("Dear ImGui OpenGL3 backend initialization failed");
		rendererReady_ = true;
	} catch (...) {
		if (rendererReady_) {
			ImGui_ImplOpenGL3_Shutdown();
			rendererReady_ = false;
		}

		ImGui::DestroyContext(context_);
		context_ = nullptr;
		throw;
	}
}

HudRenderer::~HudRenderer() {
	if (!context_)
		return;

	ImGui::SetCurrentContext(context_);
	if (rendererReady_)
		ImGui_ImplOpenGL3_Shutdown();

	ImGui::DestroyContext(context_);
	context_ = nullptr;
	rendererReady_ = false;
}

HudActions HudRenderer::takeActions() {
	const HudActions result = actions_;
	actions_ = {};
	return result;
}

void HudRenderer::draw(const World &world, const FrameInfo &frame) {
	ImGui::SetCurrentContext(context_);
	const float width = static_cast<float>(frame.width);
	const float height = static_cast<float>(frame.height);

	drawPlayerCards(world, width, height);

	if (world.phase() == Phase::Playing) {
		ImGui::SetNextWindowPos(ImVec2(width * 0.5f, 12.0f), ImGuiCond_Always,
		                        ImVec2(0.5f, 0.0f));
		const ImVec2 digits = ImGui::CalcTextSize("00:00");
		const ImVec2 pad = ImGui::GetStyle().WindowPadding;

		ImGui::SetNextWindowSize({digits.x + 2 * pad.x, digits.y + 2 * pad.y});

		if (ImGui::Begin("##round_timer", nullptr, passive)) {
			const int seconds = std::max(0, static_cast<int>(world.roundTime()));
			ImGui::Text("%02d:%02d", seconds / 60, seconds % 60);
		}

		ImGui::End();
	}

	if (!frame.paused && world.phase() != Phase::Playing) {
		const char *title = world.phase() == Phase::Ready ? "Ready to deploy###round"
		                    : world.phase() == Phase::Countdown ? "Deployment###round"
		                    : world.phase() == Phase::MatchOver
		                        ? "Match complete###round"
		                        : "Round complete###round";
		ImGui::SetNextWindowPos(ImVec2(width * 0.5f, height * 0.5f), ImGuiCond_Always,
		                        ImVec2(0.5f, 0.5f));
		ImGui::SetNextWindowSize(
		    {240.0f, world.phase() == Phase::MatchOver ? 90.0f : 68.0f});
		if (ImGui::Begin(title, nullptr, anchored)) {
			if (world.phase() == Phase::Ready) {
				if (ImGui::Button("Deploy"))
					actions_.deploy = true;
			} else if (world.phase() == Phase::Countdown) {
				const int seconds = static_cast<int>(std::ceil(std::max(
				    0.0f, world.config().rules.countdownSeconds - world.phaseTime())));
				ImGui::Text("Starting in %d", seconds);
			} else {
				const TeamId winner = world.phase() == Phase::MatchOver
				                          ? world.matchWinner()
				                          : world.roundWinner();

				if (winner == noTeam)
					ImGui::TextUnformatted("Draw");
				else if (winner < world.config().teams.size())
					ImGui::Text("%s wins", world.config().teams[winner].name.c_str());
				if (world.phase() == Phase::MatchOver && ImGui::Button("Restart match"))
					actions_.restart = true;
			}
		}

		ImGui::End();
	}

	if (frame.paused) {
		ImGui::SetNextWindowPos(ImVec2(width * 0.5f, height * 0.5f), ImGuiCond_Always,
		                        ImVec2(0.5f, 0.5f));
		ImGui::SetNextWindowSize({240.0f, 64.0f});

		if (ImGui::Begin("Paused", nullptr, anchored)) {
			if (ImGui::Button("Resume", ImVec2(100.0f, 0)))
				actions_.togglePause = true;

			ImGui::SameLine();

			if (ImGui::Button("Restart", ImVec2(100.0f, 0)))
				actions_.restart = true;
		}

		ImGui::End();
	}

	ImGui::Render();

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
} // namespace tanks::graphics
