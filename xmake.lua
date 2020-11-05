--
-- server
--
target("door_server")
  set_kind("binary")
  set_targetdir("bin")
  set_objectdir("build/door_server")
  set_languages("c99", "cxx14")
  add_files("src/share/**.cpp")
  add_files("src/server/**.cpp", "src/server/**.c")
  add_links("spdlog", "pthread", "curl", "jsoncpp", "wiringPi")
  --add_defines("__DEV__")


--
-- client
--
target("door_client")
  set_kind("binary")
  set_targetdir("bin")
  set_objectdir("build/door_client")
  set_languages("c99", "cxx14")
  add_files("src/share/**.cpp")
  add_files("src/client/**.cpp", "src/client/**.c")
  add_links("spdlog", "pthread", "curl", "jsoncpp", "wiringPi")
  --add_defines("__DEV__")

