//
// Created by Glodxy on 2021/11/26.
//

#include "include/Texture.h"
#include "ResourceAllocator.h"
#include "Backend/include/BackendUtils.h"
#include "Backend/include/DriverEnum.h"
#include <array>

namespace our_graph {
struct Texture::Detail {
  Driver* driver = nullptr;
  uint32_t width = 1; // 宽
  uint32_t height = 1; // 高
  uint32_t depth = 1; // 深度
  uint8_t levels = 1; // mipmap级别
  Sampler sampler = Sampler::SAMPLER_2D;
  InternalFormat format = InternalFormat::RGBA8;
  Usage usage = Usage::DEFAULT;
  bool texture_is_swizzle = false;
  std::array<Swizzle, 4> swizzles = {
      Swizzle ::CHANNEL_0, Swizzle ::CHANNEL_1,
      Swizzle::CHANNEL_2, Swizzle::CHANNEL_3
  };
};


using BuilderType = Texture;
BuilderType::Builder::Builder(Driver* driver) noexcept {
  impl_->driver = driver;
}
BuilderType::Builder::~Builder() noexcept = default;
BuilderType::Builder::Builder(BuilderType::Builder const& rhs) noexcept = default;
BuilderType::Builder::Builder(BuilderType::Builder&& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(BuilderType::Builder const& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(BuilderType::Builder&& rhs) noexcept = default;

Texture::Builder & Texture::Builder::Width(uint32_t width) noexcept {
  impl_->width = width;
  return *this;
}

Texture::Builder & Texture::Builder::Height(uint32_t height) noexcept {
  impl_->height = height;
  return *this;
}

Texture::Builder & Texture::Builder::Depth(uint32_t depth) noexcept {
  impl_->depth = depth;
  return *this;
}

Texture::Builder & Texture::Builder::Levels(uint8_t levels) noexcept {
  impl_->levels = levels;
  return *this;
}

Texture::Builder & Texture::Builder::Sampler(Texture::Sampler target) noexcept {
  impl_->sampler = target;
  return *this;
}

Texture::Builder & Texture::Builder::Format(InternalFormat format) noexcept {
  impl_->format = format;
  return *this;
}

Texture::Builder & Texture::Builder::Usage(Texture::Usage usage) noexcept {
  impl_->usage = usage;
  return *this;
}

Texture::Builder & Texture::Builder::Swizzle(
    Texture::Swizzle r, Texture::Swizzle g,
    Texture::Swizzle b, Texture::Swizzle a) noexcept {
  impl_->texture_is_swizzle = true;
  impl_->swizzles = {r, g, b, a};
  return *this;
}

Texture *Texture::Builder::Build() {
  if (!IsTextureFormatSupported(impl_->format)) {
    LOG_ERROR("Texture", "{} Format not supported!", impl_->format);
    return nullptr;
  }
  // 判断能否采样
  const bool samplerable = bool(impl_->usage & TextureUsage::SAMPLEABLE);
  const bool swizzled = impl_->texture_is_swizzle;
  if (swizzled && !samplerable) {
    LOG_ERROR("Texture", "Swizzled texture must be samplerable!");
    return nullptr;
  }

  return ResourceAllocator::Get().CreateTexture(*this);
}

Texture::Texture(const Builder &builder) :
    width_(builder->width), height_(builder->height),
    format_(builder->format), usage_(builder->usage),
    target_(builder->sampler), depth_(builder->depth),
    driver_(builder->driver) {
  level_cnt_ = std::min(builder->levels, Texture::GetMaxLevel(width_, height_));
  if (!builder->texture_is_swizzle) {
    handle_ = driver_->CreateTexture(target_, level_cnt_, format_,
                                     sample_cnt, width_, height_,
                                     depth_, usage_);
  } else {
    LOG_ERROR("Texture", "Swizzle texture not supported!");
  }
}

void Texture::Destroy() {
  driver_->DestroyTexture(handle_);
}

size_t Texture::GetWidth(size_t level) const noexcept {
  return ValueOfLevel(level, width_);
}

size_t Texture::GetHeight(size_t level) const noexcept {
  return ValueOfLevel(level, height_);
}

size_t Texture::GetDepth(size_t level) const noexcept {
  return ValueOfLevel(level, depth_);
}

size_t Texture::GetLevels() const noexcept {
  return level_cnt_;
}

size_t Texture::GetMaxLevels() const noexcept {
  return GetMaxLevel(width_, height_);
}

Sampler Texture::GetSamplerType() const noexcept {
  return target_;
}

InternalFormat Texture::GetFormat() const noexcept {
  return format_;
}

Usage Texture::GetUsage() const noexcept {
  return usage_;
}

bool Texture::IsCubemap() const {
  return target_ == Sampler::SAMPLER_CUBEMAP;
}

bool Texture::IsMultiSample() const {
  return sample_cnt > 1;
}

bool Texture::IsCompressed() const {
  return IsCompressedFormat(format_);
}

size_t Texture::ValueOfLevel(uint8_t level, size_t base_level_value) {
  return std::max(size_t(1), base_level_value >> level);
}

uint8_t Texture::GetMaxLevel(uint32_t max_dimension) {
  return std::max(1, std::ilogbf(max_dimension) + 1);
}

uint8_t Texture::GetMaxLevel(uint32_t width, uint32_t height) {
  return std::max(1, std::ilogbf(std::max(width, height)) + 1);
}



bool Texture::IsTextureFormatSupported(InternalFormat format) noexcept {
  // todo
  return true;
}

bool Texture::IsTextureSwizzleSupported() noexcept {
  // todo
  return true;
}

size_t Texture::ComputeTextureDataSize(
    Format format, Type type, size_t stride,
    size_t height, size_t alignment) noexcept {
  return PixelBufferDescriptor ::ComputeDataSize(format, type, stride, height, alignment);
}

size_t Texture::GetFormatSize(InternalFormat format) {
  return BackendUtil::GetFormatSize(format);
}

void Texture::SetImage(size_t level,
                       uint32_t x_offset,
                       uint32_t y_offset,
                       uint32_t width,
                       uint32_t height,
                       PixelBufferDescriptor &&buffer) const {
  auto ValidateTarget = [](SamplerType sampler) -> bool {
    switch (sampler) {
      case SamplerType::SAMPLER_2D:
      case SamplerType::SAMPLER_EXTERNAL:
        return true;
      case SamplerType::SAMPLER_CUBEMAP:
      case SamplerType::SAMPLER_3D:
      case SamplerType::SAMPLER_2D_ARRAY:
        return false;
    }
  };

  if (buffer.type_ == PixelDataType::COMPRESSED) {
    LOG_ERROR("Texture", "Compressed texture not supported!");
    return;
  }

  if (level >= level_cnt_) {
    LOG_ERROR("Texture", "{} is >= to level_cnt {}",
              unsigned(level), level_cnt_);
    return;
  }

  if (!ValidateTarget(target_)) {
    LOG_ERROR("Texture", "Sampler Type {} not supported for update2DImage", target_);
    return;
  }

  if (!buffer.buffer_) {
    LOG_ERROR("Texture", "Data buffer is null!");
    return;
  }

  if (sample_cnt > 1) {
    LOG_ERROR("Texture", "update2DImage not supported multisample {}!", sample_cnt);
    return;
  }

  if (x_offset + width > ValueOfLevel(level, width_)) {
    LOG_ERROR("Texture", "xoffset[{}] + width[{}] > texture width[{}] of level[{}]",
              x_offset, width, ValueOfLevel(level, width_), level);
    return;
  }

  if (y_offset + height > ValueOfLevel(level, height_)) {
    LOG_ERROR("Texture", "xyffset[{}] + height[{}] > texture height[{}] of level[{}]",
              y_offset, height, ValueOfLevel(level, height_), level);
    return;
  }

  driver_->Update2DImage(handle_, uint8_t(level), x_offset, y_offset,
                         width, height, std::move(buffer));
}


}