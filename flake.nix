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
        };
        pkgs = pkgs_.__splicedPackages;
      in {
        default = pkgs.gccStdenv.mkDerivation {
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
