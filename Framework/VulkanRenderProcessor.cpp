//
// Created by Glodxy on 2021/8/28.
//

#include "VulkanRenderProcessor.h"

#include "Backend/Vulkan/VulkanDriver.h"
#include "DriverContext.h"
#if __APPLE__
#include "Backend/Vulkan/VulkanPlatformMacos.h"
#elif WIN32
#include "Backend/Vulkan/VulkanPlatformWindows.h"
#endif
#include "Backend/Vulkan/VulkanSwapChain.h"
namespace our_graph {
void VulkanRenderProcessor::Init() {
  std::unique_ptr<IPlatform> platform;
#if __APPLE__
  platform = std::make_unique<VulkanPlatformMacos>();
#elif WIN32
  platform = std::make_unique<VulkanPlatformWindows>();
#endif
  driver_ = new VulkanDriver();
  driver_->Init(std::move(platform));
 }

void VulkanRenderProcessor::Destroy() {
  driver_->DestroyRenderTarget(rth_);
  driver_->DestroySwapChain(sch_);

  driver_->Clear();
  delete driver_;
}

void VulkanRenderProcessor::End() {
}

void VulkanRenderProcessor::Start() {
  sch_ = driver_->CreateSwapChain(DriverContext::Get().window_handle_, 0);
  rth_ = driver_->CreateDefaultRenderTarget();
}


void VulkanRenderProcessor::AfterRender() {
  driver_->Commit(sch_);

  driver_->EndFrame(frame++);
  driver_->Tick();
}

void VulkanRenderProcessor::BeforeRender() {
  driver_->MakeCurrent(sch_, sch_);
  driver_->Tick();
  uint64_t time = std::chrono::system_clock::now().time_since_epoch().count();
  driver_->BeginFrame(time, frame);
}

void VulkanRenderProcessor::Render() {
  RenderPassParams params;
  float r = (frame%100)/100.f;
  float g = (frame % 170)/170.f;
  float b = (frame % 190)/190.f;
  params.clearColor = glm::vec4(r, g, b, 1.f);
  params.flags.clear = TargetBufferFlags::COLOR;
  params.viewport.width = 1920;
  params.viewport.height = 1080;
  driver_->BeginRenderPass(rth_, params);
  driver_->EndRenderPass();
}

}  // namespace our_graph