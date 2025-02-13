#include <hex/api/localization_manager.hpp>

#include <imgui.h>
#include <hex/ui/imgui_imhex_extensions.h>

#include <hex/helpers/http_requests.hpp>
#include <hex/helpers/utils.hpp>

#include <chrono>
#include <fonts/vscode_icons.hpp>

namespace hex::plugin::builtin {

    using namespace std::literals::chrono_literals;

    void drawHTTPRequestMaker() {
        static std::string url, body, output, responseText;
        static std::vector<std::pair<std::string, std::string>> headers;
        static int method = 0;
        static HttpRequest request("", "");
        static std::future<HttpRequest::Result<std::string>> response;

        constexpr static auto Methods = std::array{
            "GET",
            "POST",
            "PUT",
            "PATCH",
            "DELETE",
            "HEAD",
            "OPTIONS",
            "CONNECT",
            "TRACE"
        };
        ImGui::SetNextItemWidth(100_scaled);
        ImGui::Combo("##method", &method, Methods.data(), Methods.size());

        ImGui::SameLine();

        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 75_scaled);
        if (ImGui::InputTextWithHint("##url", "hex.builtin.tools.http_requests.enter_url"_lang, url)) {
            output.clear();
        }

        ImGui::SameLine();

        ImGui::SetNextItemWidth(75_scaled);
        if (ImGui::Button("hex.builtin.tools.http_requests.send"_lang)) {
            request.setMethod(Methods[method]);
            request.setUrl(url);
            request.setBody(body);

            for (const auto &[key, value] : headers)
                request.addHeader(key, value);

            response = request.execute<std::string>();
        }

        if (ImGui::BeginChild("Settings", ImVec2(ImGui::GetContentRegionAvail().x, 200_scaled), ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
            if (ImGui::BeginTabBar("SettingsTabs")) {
                if (ImGui::BeginTabItem("hex.builtin.tools.http_requests.headers"_lang)) {
                    if (ImGui::BeginTable("Headers", 3, ImGuiTableFlags_Borders, ImGui::GetContentRegionAvail() - ImVec2(0, ImGui::GetTextLineHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y * 2))) {
                        ImGui::TableSetupColumn("hex.ui.common.key"_lang, ImGuiTableColumnFlags_NoSort);
                        ImGui::TableSetupColumn("hex.ui.common.value"_lang, ImGuiTableColumnFlags_NoSort);
                        ImGui::TableSetupColumn("##remove", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed, 20_scaled);
                        ImGui::TableSetupScrollFreeze(0, 1);

                        ImGui::TableHeadersRow();

                        auto elementToRemove = headers.end();
                        for (auto it = headers.begin(); it != headers.end(); ++it) {
                            auto &[key, value] = *it;

                            ImGui::TableNextRow();
                            ImGui::PushID(&key);

                            ImGui::TableNextColumn();
                            ImGui::SetNextItemWidth(-1);
                            ImGui::InputTextWithHint("##key", "hex.ui.common.key"_lang, key);

                            ImGui::TableNextColumn();
                            ImGui::SetNextItemWidth(-1);
                            ImGui::InputTextWithHint("##value", "hex.ui.common.value"_lang, value);

                            ImGui::TableNextColumn();
                            if (ImGuiExt::IconButton(ICON_VS_REMOVE, ImGui::GetStyleColorVec4(ImGuiCol_Text)))
                                elementToRemove = it;

                            ImGui::PopID();
                        }

                        if (elementToRemove != headers.end())
                            headers.erase(elementToRemove);

                        ImGui::TableNextColumn();

                        ImGui::Dummy(ImVec2(0, 0));

                        ImGui::EndTable();
                    }
                    if (ImGuiExt::DimmedButton("hex.ui.common.add"_lang)) headers.emplace_back();

                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("hex.builtin.tools.http_requests.body"_lang)) {
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetStyleColorVec4(ImGuiCol_ChildBg));
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1_scaled);
                    ImGui::InputTextMultiline("##comment", body, ImVec2(ImGui::GetContentRegionAvail().x, 150_scaled));
                    ImGui::PopStyleVar();
                    ImGui::PopStyleColor();
                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }
        }
        ImGui::EndChild();

        ImGuiExt::Header("hex.builtin.tools.http_requests.response"_lang);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetStyleColorVec4(ImGuiCol_ChildBg));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1_scaled);
        ImGui::InputTextMultiline("##comment", responseText, ImVec2(ImGui::GetContentRegionAvail().x, 150_scaled), ImGuiInputTextFlags_ReadOnly);
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();

        if (response.valid() && response.wait_for(0s) != std::future_status::timeout) {
            const auto result = response.get();
            const auto data = result.getData();

            if (const auto status = result.getStatusCode(); status != 0)
                responseText = "Status: " + std::to_string(result.getStatusCode()) + "\n\n" + data;
            else
                responseText = "Status: No Response";
        }

    }

}
