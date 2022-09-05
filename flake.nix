{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs";
  };
  outputs = {
    self,
    nixpkgs,
  }: let
    forSystems = nixpkgs.lib.genAttrs nixpkgs.lib.systems.flakeExposed;
  in {
    devShells = forSystems (
      system: let
        pkgs_ = import nixpkgs {
          localSystem.system = system;
          crossSystem.system = "i686-linux";
          overlays = [
            (final: prev: {
              SDL2 = prev.SDL2.override {
                # We aren't actually using SDL2 from here, but we need to trick meson into using the headers and soname
                alsaSupport = false;
                dbusSupport = false;
                libdecorSupport = false;
                pipewireSupport = false;
                pulseaudioSupport = false;
                udevSupport = false;
                waylandSupport = false;
                x11Support = false;
                drmSupport = false;
              };
            })
          ];
        };
        pkgs = pkgs_.__splicedPackages;
      in {
        default = pkgs.stdenv.mkDerivation {
          name = "devshell";
          # For systemd-coredump
          SYSTEMD_DEBUGGER = "lldb";
          # Make vcpkg x264 happy. THIS DOESN'T WORK!
          AS = "nasm";
          hardeningDisable = [ "all" ];
          buildInputs = with pkgs; [
            libGL
          ];
          nativeBuildInputs = with pkgs; [
            # For build
            cmake
            ninja
            pkg-config

            # vcpkg
            curl
            zip
            unzip
            gnutar

            # elfutils
            flex
            bison
            autoconf
            automake
            gettext

            # Debugging
            lldb

            # x264
            nasm
          ];
        };
      }
    );
  };
}
