
add_requires("libsdl","libpng", "glib", "freetype", "openal")

add_rules("mode.debug", "mode.release")

target("86box-lib")
	set_kind("static")
	set_languages("gnu11", "gnu++11")
	add_packages("libsdl","libpng", "glib", "freetype")

	-- common compiler flags
	add_cxflags("-Wpedantic", "-Wall", "-Wextra", "-Wno-deprecated", "-Wno-deprecated-declarations", "-march=native")
	add_ldflags("-march=native")

	if is_mode("debug") then
        add_defines("DEBUG")
		add_cxflags("-fno-omit-frame-pointer")
		add_ldflags("-fno-omit-frame-pointer")
    else is_mode("release")
        add_defines("RELEASE")
		add_cxflags("-flto")
		add_ldflags("-flto")
    end

	-- recursively search for .c and .cpp files under the src directory
	-- but exclude any .c or .cpp files under src/minitrace/, src/win/, floppy/lzf/ directories and the main.c file
	-- also exclude any dev branch files for the meantime
	add_files("src/**.c|minitrace/**.c|floppy/lzf/**.c|codegen_new/**.c|codegen/**.c|win/**.c")
	add_files("src/**.cpp|minitrace/**.cpp|floppy/lzf/**.cpp|win/**.cpp")
	del_files("src/main.c")

	-- compile correct codegen files
	-- if is_arch("x86_64") then
	-- 	add_files("src/codegen/**x86-64.c")
	-- end
	-- if is_arch("i386") then
    -- 	add_files("src/codegen/**x86.c")
	-- end
	-- begin excluding dev branch files from build
	del_files("src/machine/m_xt_laserxt.c", "src/video/vid_mga.c",
		"src/sound/snd_pas16.c", "src/sio/sio_detect.c", "src/chipset/ali1489.c",
		"src/chipset/ali1531.c", "src/chipset/ali1543.c", "src/chipset/ali6117.c",
		"src/chipset/sis_5571.c")

	del_files("src/network/pcap_if.c")

	-- if target is Windows then add the Windows source files, except the win.c file because it contains WinMain()
	if is_os("windows") then
		add_files("src/win/**.cpp", "src/win/**.c|src/win/win.c")
		add_defines("WIN32", "_WIN32")
	end

	add_includedirs("src/include")
	add_includedirs("src/cpu")
	add_includedirs("src/include_make")
	add_includedirs("src/codegen")
target_end()

target("86box")
	set_kind("binary")
	set_languages("gnu11", "gnu++11")
	add_deps("86box-lib")
	add_packages("libsdl","libpng", "glib", "freetype")
	set_rundir("$(projectdir)")

	-- common compiler flags
	add_cxflags("-Wpedantic", "-Wall", "-Wextra", "-Wno-deprecated", "-Wno-deprecated-declarations", "-march=native")
	add_ldflags("-march=native")

	if is_mode("debug") then
        add_defines("DEBUG")
		add_cxflags("-fno-omit-frame-pointer")
		add_ldflags("-fno-omit-frame-pointer")
    else is_mode("release")
        add_defines("RELEASE")
		add_cxflags("-flto")
		add_ldflags("-flto")
    end

	-- only need the main.c because the rest is in the 86box-lib dependency
	add_files("src/main.c")

	-- if target is Windows then use the win.c file for WinMain()
	if is_os("windows") then
		add_files("src/win/win.c")
		add_defines("WIN32", "_WIN32")
	end

	add_includedirs("src/include")
	add_includedirs("src/cpu")
	add_includedirs("src/include_make")
	add_includedirs("src/codegen")
target_end()
