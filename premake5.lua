workspace "WindowsSupervisionSuite"
    configurations { "Debug", "Release" }
    platforms { "x64" }
    language "C++"
    location "build"
    characterset "MBCS"
    symbols "On"
    flags { "NoPCH", "FatalWarnings" }
    defines {
      "VC_EXTRALEAN",
      "WINS32_LEAN_AND_MEAN",
      'PROJECT_NAME="$(ProjectName)"',
      'YEAR=' .. os.date("%Y")
    }

    build_version = os.getenv("BUILD_VERSION")
    if not build_version then
      build_version = "1.0.0.0"
    end

    parts = string.explode(build_version, '[.-]+')
    if (table.getn(parts) >= 1) then
      defines("VERSION_MAJOR=" .. parts[1] .. "")
    end
    if (table.getn(parts) >= 2) then
      defines("VERSION_MINOR=" .. parts[2] .. "")
    end
    if (table.getn(parts) >= 3) then
      defines("VERSION_REVISION=" .. parts[3] .. "")
    end
    if (table.getn(parts) >= 4) then
      defines("VERSION_BUILD=" .. parts[4] .. "")
    end

    git_commit = os.getenv("GIT_COMMIT")
    if git_commit then
      defines("GIT_COMMIT=\"" .. git_commit .. "\"")
      defines("GIT_COMMIT_SHORT=\"" .. string.sub(git_commit, 1, 8) .. "\"")
    end

    project "winss"
      kind "StaticLib"
      includedirs { "lib" }
      files { "lib/**.hpp", "lib/**.cpp" }

    project "winss-supervise"
      kind "ConsoleApp"
      links { "winss" }
      includedirs { "lib" }
      files { "bin/winss-supervise.cpp", "bin/resource/*" }

    project "winss-svc"
      kind "ConsoleApp"
      links { "winss" }
      includedirs { "lib" }
      files { "bin/winss-svc.cpp", "bin/resource/*" }

    project "winss-svok"
      kind "ConsoleApp"
      links { "winss" }
      includedirs { "lib" }
      files { "bin/winss-svok.cpp", "bin/resource/*" }

    project "winss-svwait"
      kind "ConsoleApp"
      links { "winss" }
      includedirs { "lib" }
      files { "bin/winss-svwait.cpp", "bin/resource/*" }

    project "winss-svstat"
      kind "ConsoleApp"
      links { "winss" }
      includedirs { "lib" }
      files { "bin/winss-svstat.cpp", "bin/resource/*" }

    project "winss-svscan"
      kind "ConsoleApp"
      links { "winss" }
      includedirs { "lib" }
      files { "bin/winss-svscan.cpp", "bin/resource/*" }

    project "winss-svscanctl"
      kind "ConsoleApp"
      links { "winss" }
      includedirs { "lib" }
      files { "bin/winss-svscanctl.cpp", "bin/resource/*" }

    project "winss-log"
      kind "ConsoleApp"
      links { "winss" }
      includedirs { "lib" }
      files { "bin/winss-log.cpp", "bin/resource/*" }

    project "winss-test"
      kind "ConsoleApp"
      links { "winss" }
      includedirs { "lib" }
      nuget { "gmock:1.7.0" }
      files { "test/**", "bin/resource/*" }
      buildoptions { "/bigobj" }
      linkoptions { "/OPT:NOREF", "/OPT:NOICF", "/INCREMENTAL:NO" }
