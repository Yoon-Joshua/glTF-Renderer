#include "scene_widget.h"

#include <stack>

SceneWidget::SceneWidget(RendererInUI& r) : renderer(r) {}

void SceneWidget::show() {
  sg::Scene& scene = renderer.get_scene();

  std::stack<sg::Node*> node_stack;

  sg::Node& root = scene.get_root_node();
  node_stack.push(&root);

  ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow |
                                  ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                  ImGuiTreeNodeFlags_SpanAvailWidth;
  node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

  ImGui::Begin(title.c_str());
  while (!node_stack.empty()) {
    sg::Node* cur_node = node_stack.top();
    node_stack.pop();

    // 空指针表示这是scene tree中的某一个层级的末尾。
    if (cur_node == nullptr) {
      ImGui::TreePop();
      continue;
    }

    std::string id = std::to_string(cur_node->get_id());
    if (ImGui::TreeNode(id.c_str())) {
      node_stack.push(nullptr);
      for (sg::Node* node : cur_node->get_children()) {
        node_stack.push(node);
      }
      if (cur_node->has_component<sg::Mesh>()) {
        ImGui::Text(cur_node->get_component<sg::Mesh>().get_name().c_str());
        ImGui::SameLine();
        ImGui::SmallButton("display");
      }
      if (cur_node->has_component<sg::Camera>()) {
        int i = 0;
        ImGui::TreeNodeEx((void*)(intptr_t)i, node_flags, "Camera");
      }
      if (cur_node->has_component<sg::Light>()) {
        ImGui::Text(cur_node->get_component<sg::Light>().get_name().c_str());
        ImGui::SameLine();
        ImGui::SmallButton("display");
      }
    }
  }

  ImGui::End();
}