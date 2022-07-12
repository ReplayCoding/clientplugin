with import <nixpkgs> {
  system = "i686-linux";
};
mkShell {
  nativeBuildInputs = [ meson ninja ];
}
