//
// Created by Glodxy on 2021/9/6.
//

#include "VulkanTextureView.h"
#include "Utils/OGLogging.h"

our_graph::VulkanTextureView::VulkanTextureView(VkDevice device,
                                                VkImageViewCreateInfo create_info) {
  device_ = device;
  create_info_ = create_info;
}

void our_graph::VulkanTextureView::Create(std::shared_ptr<IBuffer> buffer) {
  if (!CreateImageView()) {
    LOG_ERROR("VulkanTextureView", "Create ImageView Failed!");
  }
  LOG_INFO("VulkanTextureView", "CreateImageViewSuc!");
}

bool our_graph::VulkanTextureView::CreateImageView() {
  VkResult view_create_res = vkCreateImageView(device_, &create_info_,
                                               nullptr, &image_view_);
  if (view_create_res != VK_SUCCESS) {
    LOG_ERROR("CreateImage", "create view failed! "
                             "res:{}", view_create_res);
    return false;
  }
  return true;
}

void our_graph::VulkanTextureView::Destroy() {
  vkDestroyImageView(device_, image_view_, nullptr);
}

void our_graph::VulkanTextureView::BindBuffer(std::shared_ptr<IBuffer> buffer) {

}