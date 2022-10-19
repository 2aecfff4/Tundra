#pragma once
#include "core/core.h"

namespace tundra::rhi {

class CommandEncoder;
class ValidationLayers;

void validate_command_encoder(
    ValidationLayers* validation_layers, const CommandEncoder& encoder) noexcept;

} // namespace tundra::rhi
