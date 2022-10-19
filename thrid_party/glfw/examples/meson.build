  # link_libraries('glfw')
  # include_directories('${GLFW_SOURCE_DIR}/deps')
  if 'MATH_LIBRARY'
    # link_libraries('${MATH_LIBRARY}')
  endif
  # Workaround for the MS CRT deprecating parts of the standard library
  if 'MSVC', 'OR', 'CMAKE_C_SIMULATE_ID', 'STREQUAL', 'MSVC'
    # add_definitions('-D_CRT_SECURE_NO_WARNINGS')
  endif
  if 'WIN32'
    icon = 'glfw.rc'
  elif 'APPLE'
    icon = 'glfw.icns'
  endif
  if cmake_version, 'VERSION_EQUAL', '3.1.0', 'OR', cmake_version, 'VERSION_GREATER', '3.1.0'
    cmake_c_standard = '99'
  else
    # Remove this fallback when removing support for CMake version less than 3.1
    # add_compile_options(['$<$<C_COMPILER_ID:AppleClang>:-std=c99>', '$<$<C_COMPILER_ID:Clang>:-std=c99>', '$<$<C_COMPILER_ID:GNU>:-std=c99>'])
  endif
  glad_gl = ['${GLFW_SOURCE_DIR}/deps/glad/gl.h', '${GLFW_SOURCE_DIR}/deps/glad_gl.c']
  getopt = ['${GLFW_SOURCE_DIR}/deps/getopt.h', '${GLFW_SOURCE_DIR}/deps/getopt.c']
  tinycthread = ['${GLFW_SOURCE_DIR}/deps/tinycthread.h', '${GLFW_SOURCE_DIR}/deps/tinycthread.c']
  boing_exe = executable('boing', 'WIN32', 'MACOSX_BUNDLE', 'boing.c', icon, glad_gl)
  gears_exe = executable('gears', 'WIN32', 'MACOSX_BUNDLE', 'gears.c', icon, glad_gl)
  heightmap_exe = executable('heightmap', 'WIN32', 'MACOSX_BUNDLE', 'heightmap.c', icon, glad_gl)
  offscreen_exe = executable('offscreen', 'offscreen.c', icon, glad_gl)
  particles_exe = executable('particles', 'WIN32', 'MACOSX_BUNDLE', 'particles.c', icon, tinycthread, getopt, glad_gl)
  sharing_exe = executable('sharing', 'WIN32', 'MACOSX_BUNDLE', 'sharing.c', icon, glad_gl)
  simple_exe = executable('simple', 'WIN32', 'MACOSX_BUNDLE', 'simple.c', icon, glad_gl)
  splitview_exe = executable('splitview', 'WIN32', 'MACOSX_BUNDLE', 'splitview.c', icon, glad_gl)
  wave_exe = executable('wave', 'WIN32', 'MACOSX_BUNDLE', 'wave.c', icon, glad_gl)
  # target_link_libraries(['particles', '${CMAKE_THREAD_LIBS_INIT}'])
  if 'RT_LIBRARY'
    # target_link_libraries(['particles', '${RT_LIBRARY}'])
  endif
  gui_only_binaries = ['boing', 'gears', 'heightmap', 'particles', 'sharing', 'simple', 'splitview', 'wave']
  console_binaries = 'offscreen'
  # set_target_properties([gui_only_binaries, console_binaries, 'PROPERTIES', 'FOLDER', 'GLFW3/Examples'])
  if 'GLFW_USE_OSMESA'
    # target_compile_definitions(['offscreen', 'PRIVATE', 'USE_NATIVE_OSMESA'])
  endif
  if 'MSVC'
    # Tell MSVC to use main instead of WinMain
    # set_target_properties([gui_only_binaries, 'PROPERTIES', 'LINK_FLAGS', '/ENTRY:mainCRTStartup'])
  elif 'CMAKE_C_SIMULATE_ID', 'STREQUAL', 'MSVC'
    # Tell Clang using MS CRT to use main instead of WinMain
    # set_target_properties([gui_only_binaries, 'PROPERTIES', 'LINK_FLAGS', '-Wl,/entry:mainCRTStartup'])
  endif
  if 'APPLE'
    # set_target_properties(['boing', 'PROPERTIES', 'MACOSX_BUNDLE_BUNDLE_NAME', 'Boing'])
    # set_target_properties(['gears', 'PROPERTIES', 'MACOSX_BUNDLE_BUNDLE_NAME', 'Gears'])
    # set_target_properties(['heightmap', 'PROPERTIES', 'MACOSX_BUNDLE_BUNDLE_NAME', 'Heightmap'])
    # set_target_properties(['particles', 'PROPERTIES', 'MACOSX_BUNDLE_BUNDLE_NAME', 'Particles'])
    # set_target_properties(['sharing', 'PROPERTIES', 'MACOSX_BUNDLE_BUNDLE_NAME', 'Sharing'])
    # set_target_properties(['simple', 'PROPERTIES', 'MACOSX_BUNDLE_BUNDLE_NAME', 'Simple'])
    # set_target_properties(['splitview', 'PROPERTIES', 'MACOSX_BUNDLE_BUNDLE_NAME', 'SplitView'])
    # set_target_properties(['wave', 'PROPERTIES', 'MACOSX_BUNDLE_BUNDLE_NAME', 'Wave'])
    # set_source_files_properties(['glfw.icns', 'PROPERTIES', 'MACOSX_PACKAGE_LOCATION', 'Resources'])
    # set_target_properties([gui_only_binaries, 'PROPERTIES', 'MACOSX_BUNDLE_SHORT_VERSION_STRING', glfw_version, 'MACOSX_BUNDLE_LONG_VERSION_STRING', glfw_version, 'MACOSX_BUNDLE_ICON_FILE', 'glfw.icns', 'MACOSX_BUNDLE_INFO_PLIST', '${GLFW_SOURCE_DIR}/CMake/MacOSXBundleInfo.plist.in'])
  endif
