{
  description = "chip-8 interpreter development environment";

  inputs = {
    nixpkgs.url = "nixpkgs/nixpkgs-unstable";
    flake-utils = { url = "github:numtide/flake-utils"; };
  };

  outputs = {self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };

      in {
        devShell = pkgs.mkShell {
          nativeBuildInputs = with pkgs; [
            gcc gnumake
          ];

          buildInputs = with pkgs; [
            ncurses
          ];
        };
      }
    );
}
