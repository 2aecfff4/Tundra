cmake_minimum_required(VERSION 3.20)
project(rhi VERSION 1.0.0 LANGUAGES CXX)

# ######################################################
# Files
set(PUBLIC_HDRS
    include/rhi/commands/barrier.h
    include/rhi/commands/command_encoder.h
    include/rhi/commands/commands.h
    include/rhi/commands/dispatch_indirect.h
    include/rhi/commands/draw_indirect.h
    include/rhi/commands/push_constants.h

    include/rhi/resources/access_flags.h
    include/rhi/resources/buffer.h
    include/rhi/resources/compute_pipeline.h
    include/rhi/resources/graphics_pipeline.h
    include/rhi/resources/handle_manager.h
    include/rhi/resources/handle.h
    include/rhi/resources/index_buffer.h
    include/rhi/resources/render_pass.h
    include/rhi/resources/resource_tracker.h
    include/rhi/resources/sampler.h
    include/rhi/resources/shader.h
    include/rhi/resources/swapchain.h
    include/rhi/resources/texture_formats.inc
    include/rhi/resources/texture.h

    include/rhi/config.h
    include/rhi/enums.h
    include/rhi/queue.h
    include/rhi/rhi_context.h
    include/rhi/rhi_module.h
    include/rhi/submit_info.h
    include/rhi/validation_layers.h
)

set(PRIVATE_HDRS
    src/validation_layers/command_encoder_validator.h
)

set(SRC
    src/commands/barrier.cpp
    src/commands/command_encoder.cpp
    src/commands/commands.cpp

    src/resources/buffer.cpp
    src/resources/compute_pipeline.cpp
    src/resources/graphics_pipeline.cpp
    src/resources/render_pass.cpp
    src/resources/resource_tracker.cpp
    src/resources/sampler.cpp
    src/resources/shader.cpp
    src/resources/swapchain.cpp
    src/resources/texture.cpp

    src/validation_layers/command_encoder_validator.cpp
    src/validation_layers/validation_layers.cpp

    src/queue.cpp
    src/rhi_context.cpp
    src/rhi_module.cpp
    src/submit_info.cpp

    rhi.natvis
)

# ######################################################
# Dependencies
set(PUBLIC_DEPENDENCIES
    core
    math
    platform
)

set(PRIVATE_DEPENDENCIES
)

tndr_add_module(rhi
    SOURCES ${SRC} ${PUBLIC_HDRS} ${PRIVATE_HDRS}
    PRIVATE_DEPENDENCIES ${PRIVATE_DEPENDENCIES}
    PUBLIC_DEPENDENCIES ${PUBLIC_DEPENDENCIES}
)
