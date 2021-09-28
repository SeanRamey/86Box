#include <array>
#include "imgui.h"
#include <86box/imgui_settings_window.h>

namespace ImGuiSettingsWindow {

	bool showSettingsWindow = false;

	// forward declares
	void RenderMachineCategory();
	void RenderDisplayCategory();
	void RenderSoundCategory();
	void RenderNetworkCategory();
	void RenderPortsCategory();
	void RenderStorageControllersCategory();
	void RenderHardDisksCategory();
	void RenderFloppyCdromDrivesCategory();
	void RenderOtherRemovableDevicesCategory();
	void RenderOtherPeripheralsCategory();

	void Render() {
		ImGui::Begin("Settings", &ImGuiSettingsWindow::showSettingsWindow);

		// Left
		static int currentSettingsCategory = 0;
		{
			ImGui::BeginChild("left pane", ImVec2(150, 0), true);

			if(ImGui::Selectable("Machine")) {
				currentSettingsCategory = 0;
			} else
			if(ImGui::Selectable("Display")) {
				currentSettingsCategory = 1;
			} else
			if(ImGui::Selectable("Sound")) {
				currentSettingsCategory = 2;
			} else
			if(ImGui::Selectable("Network")) {
				currentSettingsCategory = 3;
			} else
			if(ImGui::Selectable("Ports (COM & LPT)")) {
				currentSettingsCategory = 4;
			} else
			if(ImGui::Selectable("Storage Controllers")) {
				currentSettingsCategory = 5;
			} else
			if(ImGui::Selectable("Hard Disks")) {
				currentSettingsCategory = 6;
			} else
			if(ImGui::Selectable("Floppy & CD-ROM Drives")) {
				currentSettingsCategory = 7;
			} else
			if(ImGui::Selectable("Other Removable Devices")) {
				currentSettingsCategory = 8;
			} else
			if(ImGui::Selectable("Other Peripherals")) {
				currentSettingsCategory = 9;
			}

			ImGui::EndChild();
		}
		ImGui::SameLine();

		// Right
		{
			ImGui::BeginGroup();
			ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us

			ImGui::Separator();

			switch(currentSettingsCategory) {
				case 0: RenderMachineCategory(); break;
				case 1: RenderDisplayCategory(); break;
				case 2: RenderSoundCategory(); break;
				case 3: RenderNetworkCategory(); break;
				case 4: RenderPortsCategory(); break;
				case 5: RenderStorageControllersCategory(); break;
				case 6: RenderHardDisksCategory(); break;
				case 7: RenderFloppyCdromDrivesCategory(); break;
				case 8: RenderOtherRemovableDevicesCategory(); break;
				case 9: RenderOtherPeripheralsCategory(); break;
				default: RenderMachineCategory();
			}

			ImGui::EndChild();
			if (ImGui::Button("Cancel")) {}
			ImGui::SameLine();
			if (ImGui::Button("Save")) {}
			ImGui::EndGroup();
		}

		ImGui::End();
	}

	void RenderMachineCategory() {

		//ImGui::PushItemWidth(ImGui::GetFontSize() * -12);
		//ImGui::AlignTextToFramePadding();

		// Machine Type Combo Drop Down Box
		const std::array machine_types {"8086", "286", "386"};
		static int machine_type_current = 0;
		const char* machine_type_preview_value = machine_types[machine_type_current];  // Pass in the preview value visible before opening the combo (it could be anything)
		ImGui::Text("Machine Type:");
		ImGui::SameLine();
		if (ImGui::BeginCombo("##Machine Type", machine_type_preview_value))
		{
			for (int n = 0; n < machine_types.size(); n++)
			{
				const bool is_selected = (machine_type_current == n);
				if (ImGui::Selectable(machine_types[n], is_selected))
					machine_type_current = n;

				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		// Machine Combo Drop Down
		const std::array machines {"1", "2", "3", "4"};
		static int machine_current = 0;
		const char* machine_preview_value = machines[machine_current];  // Pass in the preview value visible before opening the combo (it could be anything)
		ImGui::Text("Machine:");
		ImGui::Spacing();
		ImGui::SameLine();
		if (ImGui::BeginCombo("##Machine", machine_preview_value))
		{
			for (int n = 0; n < machines.size(); n++)
			{
				const bool is_selected = (machine_current == n);
				if (ImGui::Selectable(machines[n], is_selected))
					machine_current = n;

				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		// CPU Type Combo Drop Down
		const std::array cpu_types {"1", "2", "3", "4"};
		static int cpu_current = 0;
		const char* cpu_preview_value = cpu_types[cpu_current];  // Pass in the preview value visible before opening the combo (it could be anything)
		ImGui::Text("CPU:");
		ImGui::Spacing();
		ImGui::SameLine();
		if (ImGui::BeginCombo("##CPU Type", cpu_preview_value))
		{
			for (int n = 0; n < cpu_types.size(); n++)
			{
				const bool is_selected = (cpu_current == n);
				if (ImGui::Selectable(cpu_types[n], is_selected))
					cpu_current = n;

				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		// CPU Speed Combo Drop Down
		const std::array cpu_speed_types {"1", "2", "3", "4"};
		static int cpu_speed_current = 0;
		const char* cpu_speed_preview_value = cpu_speed_types[cpu_current];  // Pass in the preview value visible before opening the combo (it could be anything)
		ImGui::Text("Speed:");
		ImGui::Spacing();
		ImGui::SameLine();
		if (ImGui::BeginCombo("##Speed", cpu_speed_preview_value))
		{
			for (int n = 0; n < cpu_speed_types.size(); n++)
			{
				const bool is_selected = (cpu_speed_current == n);
				if (ImGui::Selectable(cpu_speed_types[n], is_selected))
					cpu_speed_current = n;

				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
	}


	void RenderDisplayCategory() {

	}

	void RenderSoundCategory() {

	}

	void RenderNetworkCategory() {

	}

	void RenderPortsCategory() {

	}

	void RenderStorageControllersCategory() {

	}

	void RenderHardDisksCategory() {

	}

	void RenderFloppyCdromDrivesCategory() {

	}

	void RenderOtherRemovableDevicesCategory() {

	}

	void RenderOtherPeripheralsCategory() {

	}

} // namespace ImGuiSettingsWindow
