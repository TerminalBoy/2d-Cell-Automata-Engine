#pragma once

#include "../../SFML/include/SFML/Graphics.hpp"
#include "../../SFML/include/SFML/OpenGL.hpp"

#include "../../ImGui/include/imgui.h"
#include "../../ImGui/include/backends/imgui_impl_opengl3.h"

namespace ImGui_SFML {

  // key mappings SFML -> ImGui
  ImGuiKey SFMLKeyToImGui(sf::Keyboard::Key key) {
    switch (key) {
    case sf::Keyboard::A: return ImGuiKey_A;
    case sf::Keyboard::B: return ImGuiKey_B;
    case sf::Keyboard::C: return ImGuiKey_C;
    case sf::Keyboard::D: return ImGuiKey_D;
    case sf::Keyboard::E: return ImGuiKey_E;
    case sf::Keyboard::F: return ImGuiKey_F;
    case sf::Keyboard::G: return ImGuiKey_G;
    case sf::Keyboard::H: return ImGuiKey_H;
    case sf::Keyboard::I: return ImGuiKey_I;
    case sf::Keyboard::J: return ImGuiKey_J;
    case sf::Keyboard::K: return ImGuiKey_K;
    case sf::Keyboard::L: return ImGuiKey_L;
    case sf::Keyboard::M: return ImGuiKey_M;
    case sf::Keyboard::N: return ImGuiKey_N;
    case sf::Keyboard::O: return ImGuiKey_O;
    case sf::Keyboard::P: return ImGuiKey_P;
    case sf::Keyboard::Q: return ImGuiKey_Q;
    case sf::Keyboard::R: return ImGuiKey_R;
    case sf::Keyboard::S: return ImGuiKey_S;
    case sf::Keyboard::T: return ImGuiKey_T;
    case sf::Keyboard::U: return ImGuiKey_U;
    case sf::Keyboard::V: return ImGuiKey_V;
    case sf::Keyboard::W: return ImGuiKey_W;
    case sf::Keyboard::X: return ImGuiKey_X;
    case sf::Keyboard::Y: return ImGuiKey_Y;
    case sf::Keyboard::Z: return ImGuiKey_Z;

    case sf::Keyboard::Space: return ImGuiKey_Space;
    case sf::Keyboard::Enter: return ImGuiKey_Enter;
    case sf::Keyboard::Escape: return ImGuiKey_Escape;

    default: return ImGuiKey_None;
    }
  }

  void MapMouseEvents(ImGuiIO& io, sf::Event& event) {
    
    if (event.type == sf::Event::MouseMoved) {
      io.AddMousePosEvent(
        static_cast<float>(event.mouseMove.x),
        static_cast<float>(event.mouseMove.y)
      );
    }

    if (event.type == sf::Event::MouseButtonPressed) {
      io.AddMouseButtonEvent(event.mouseButton.button, true);
    }

    if (event.type == sf::Event::MouseButtonReleased) {
      io.AddMouseButtonEvent(event.mouseButton.button, false);
    }

    if (event.type == sf::Event::MouseWheelScrolled) {
      io.AddMouseWheelEvent(0.0f, event.mouseWheelScroll.delta);
    }
  }

  void MapKeyboardEvents(ImGuiIO& io, sf::Event& event) {
    // keyboard
    if (event.type == sf::Event::KeyPressed) {
      ImGuiKey key = SFMLKeyToImGui(event.key.code);
      if (key != ImGuiKey_None)
        io.AddKeyEvent(key, true);
    }

    if (event.type == sf::Event::KeyReleased) {
      ImGuiKey key = SFMLKeyToImGui(event.key.code);
      if (key != ImGuiKey_None)
        io.AddKeyEvent(key, false);
    }

    // text input
    if (event.type == sf::Event::TextEntered) {
      if (event.text.unicode < 0x10000)
        io.AddInputCharacter((unsigned short)event.text.unicode);
    }

  }

  void MapFrame(ImGuiIO& io, sf::RenderWindow& window) {
    io.DisplaySize = ImVec2(
      static_cast<float>(window.getSize().x),
      static_cast<float>(window.getSize().y)
    );
  }

  void MapDeltaClock(ImGuiIO& io, float delta_time) {
    io.DeltaTime = delta_time;
  }
  
  void InitWith_DarkMode() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    ImGui::GetIO().IniFilename = nullptr;
    ImGui_ImplOpenGL3_Init("#version 130");
  }

  void InitWith_LightMode() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsLight();
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    ImGui::GetIO().IniFilename = nullptr;
    ImGui_ImplOpenGL3_Init("#version 130");
  }

  void Map(ImGuiIO& io, sf::Event& event, sf::RenderWindow& window, float delta_time) {
    MapMouseEvents(io, event);
    MapKeyboardEvents(io, event);
    MapFrame(io, window);
    MapDeltaClock(io, delta_time);
    
  }

  void ImGuiInitNewFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();
  }

  void RenderUi() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  }

  void CleanUp() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();
  }


  sf::ContextSettings SFML_StandardContext() {
    sf::ContextSettings settings;
    settings.depthBits = 24;
    settings.stencilBits = 8;
    settings.majorVersion = 3;
    settings.minorVersion = 3;
    return settings;
  }

}



