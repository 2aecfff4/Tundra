  # link_libraries('glfw')
  # include_directories('${GLFW_SOURCE_DIR}/deps')
  if 'MATH_LIBRARY'
    # link_libraries('${MATH_LIBRARY}')
  endif
  # Workaround for the MS CRT deprecating parts of the standard library
  if 'MSVC', 'OR', 'CMAKE_C_SIMULATE_ID', 'STREQUAL', 'MSVC'
    # add_definitions('-D_CRT_SECURE_NO_WARNINGS')
  endif
  glad_gl = ['${GLFW_SOURCE_DIR}/deps/glad/gl.h', '${GLFW_SOURCE_DIR}/deps/glad_gl.c']
  glad_vulkan = ['${GLFW_SOURCE_DIR}/deps/glad/vulkan.h', '${GLFW_SOURCE_DIR}/deps/glad_vulkan.c']
  getopt = ['${GLFW_SOURCE_DIR}/deps/getopt.h', '${GLFW_SOURCE_DIR}/deps/getopt.c']
  tinycthread = ['${GLFW_SOURCE_DIR}/deps/tinycthread.h', '${GLFW_SOURCE_DIR}/deps/tinycthread.c']
  if cmake_version, 'VERSION_EQUAL', '3.1.0', 'OR', cmake_version, 'VERSION_GREATER', '3.1.0'
    cmake_c_standard = '99'
  else
    # Remove this fallback when removing support for CMake version less than 3.1
    # add_compile_options(['$<$<C_COMPILER_ID:AppleClang>:-std=c99>', '$<$<C_COMPILER_ID:Clang>:-std=c99>', '$<$<C_COMPILER_ID:GNU>:-std=c99>'])
  endif
  clipboard_exe = executable('clipboard', 'clipboard.c', getopt, glad_gl)
  events_exe = executable('events', 'events.c', getopt, glad_gl)
  msaa_exe = executable('msaa', 'msaa.c', getopt, glad_gl)
  glfwinfo_exe = executable('glfwinfo', 'glfwinfo.c', getopt, glad_gl, glad_vulkan)
  iconify_exe = executable('iconify', 'iconify.c', getopt, glad_gl)
  monitors_exe = executable('monitors', 'monitors.c', getopt, glad_gl)
  reopen_exe = executable('reopen', 'reopen.c', glad_gl)
  cursor_exe = executable('cursor', 'cursor.c', glad_gl)
  empty_exe = executable('empty', 'WIN32', 'MACOSX_BUNDLE', 'empty.c', tinycthread, glad_gl)
  gamma_exe = executable('gamma', 'WIN32', 'MACOSX_BUNDLE', 'gamma.c', glad_gl)
  icon_exe = executable('icon', 'WIN32', 'MACOSX_BUNDLE', 'icon.c', glad_gl)
  inputlag_exe = executable('inputlag', 'WIN32', 'MACOSX_BUNDLE', 'inputlag.c', getopt, glad_gl)
  joysticks_exe = executable('joysticks', 'WIN32', 'MACOSX_BUNDLE', 'joysticks.c', glad_gl)
  opacity_exe = executable('opacity', 'WIN32', 'MACOSX_BUNDLE', 'opacity.c', glad_gl)
  tearing_exe = executable('tearing', 'WIN32', 'MACOSX_BUNDLE', 'tearing.c', glad_gl)
  threads_exe = executable('threads', 'WIN32', 'MACOSX_BUNDLE', 'threads.c', tinycthread, glad_gl)
  timeout_exe = executable('timeout', 'WIN32', 'MACOSX_BUNDLE', 'timeout.c', glad_gl)
  title_exe = executable('title', 'WIN32', 'MACOSX_BUNDLE', 'title.c', glad_gl)
  triangle-vulkan_exe = executable('triangle-vulkan', 'WIN32', 'triangle-vulkan.c', glad_vulkan)
  windows_exe = executable('windows', 'WIN32', 'MACOSX_BUNDLE', 'windows.c', getopt, glad_gl)
  # target_link_libraries(['empty', '${CMAKE_THREAD_LIBS_INIT}'])
  # target_link_libraries(['threads', '${CMAKE_THREAD_LIBS_INIT}'])
  if 'RT_LIBRARY'
    # target_link_libraries(['empty', '${RT_LIBRARY}'])
    # target_link_libraries(['threads', '${RT_LIBRARY}'])
  endif
  gui_only_binaries = ['empty', 'gamma', 'icon', 'inputlag', 'joysticks', 'opacity', 'tearing', 'threads', 'timeout', 'title', 'triangle-vulkan', 'windows']
  console_binaries = ['clipboard', 'events', 'msaa', 'glfwinfo', 'iconify', 'monitors', 'reopen', 'cursor']
  # set_target_properties([gui_only_binaries, console_binaries, 'PROPERTIES', 'FOLDER', 'GLFW3/Tests'])
  if 'MSVC'
    # Tell MSVC to use main instead of WinMain
    # set_target_properties([gui_only_binaries, 'PROPERTIES', 'LINK_FLAGS', '/ENTRY:mainCRTStartup'])
  elif 'CMAKE_C_SIMULATE_ID', 'STREQUAL', 'MSVC'
    # Tell Clang using MS CRT to use main instead of WinMain
    # set_target_properties([gui_only_binaries, 'PROPERTIES', 'LINK_FLAGS', '-Wl,/entry:mainCRTStartup'])
  endif
  if 'APPLE'
    # set_target_properties(['empty', 'PROPERTIES', 'MACOSX_BUNDLE_BUNDLE_NAME', 'Empty Event'])
    # set_target_properties(['gamma', 'PROPERTIES', 'MACOSX_BUNDLE_BUNDLE_NAME', 'Gamma'])
    # set_target_properties(['inputlag', 'PROPERTIES', 'MACOSX_BUNDLE_BUNDLE_NAME', 'Input Lag'])
    # set_target_properties(['joysticks', 'PROPERTIES', 'MACOSX_BUNDLE_BUNDLE_NAME', 'Joysticks'])
    # set_target_properties(['opacity', 'PROPERTIES', 'MACOSX_BUNDLE_BUNDLE_NAME', 'Opacity'])
    # set_target_properties(['tearing', 'PROPERTIES', 'MACOSX_BUNDLE_BUNDLE_NAME', 'Tearing'])
    # set_target_properties(['threads', 'PROPERTIES', 'MACOSX_BUNDLE_BUNDLE_NAME', 'Threads'])
    # set_target_properties(['timeout', 'PROPERTIES', 'MACOSX_BUNDLE_BUNDLE_NAME', 'Timeout'])
    # set_target_properties(['title', 'PROPERTIES', 'MACOSX_BUNDLE_BUNDLE_NAME', 'Title'])
    # set_target_properties(['windows', 'PROPERTIES', 'MACOSX_BUNDLE_BUNDLE_NAME', 'Windows'])
    # set_target_properties([gui_only_binaries, 'PROPERTIES', 'MACOSX_BUNDLE_SHORT_VERSION_STRING', glfw_version, 'MACOSX_BUNDLE_LONG_VERSION_STRING', glfw_version, 'MACOSX_BUNDLE_INFO_PLIST', '${GLFW_SOURCE_DIR}/CMake/MacOSXBundleInfo.plist.in'])
  endif
