//
// Created by Glodxy on 2021/9/6.
//

#ifndef OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANTEXTUREVIEW_H_
#define OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANTEXTUREVIEW_H_
#include "../include_internal/ITextureView.h"
#include "vulkan/vulkan.h"
namespace our_graph {
class VulkanTextureView : public ITextureView{
 public:
  explicit VulkanTextureView(VkDevice device,
                             VkImageViewCreateInfo create_info);
  void Create(std::shared_ptr<IBuffer> buffer) override;
  void Destroy() override;
  void BindBuffer(std::shared_ptr<IBuffer> buffer) override;

 private:
  bool CreateImageView();


  VkDevice device_;
  VkImageViewCreateInfo create_info_;
  VkImageView image_view_;
};
} // namespace our_graph
#endif //OUR_GRAPHIC_FRAMEWORK_BACKEND_VULKAN_VULKANTEXTUREVIEW_H_
