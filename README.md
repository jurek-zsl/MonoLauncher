<p align="center">
  <img alt="MonoLauncher" src="/program_info/io.github.jurek_zsl.MonoLauncher.svg" width="160">
</p>

<h1 align="center">MonoLauncher</h1>

<p align="center">
  MonoLauncher is a custom launcher for Minecraft that allows you to easily manage multiple installations of Minecraft at once.<br />
  <br />It is a <b>fork</b> of <a href="https://github.com/PrismLauncher/PrismLauncher">Prism Launcher</a> and is <b>not</b> endorsed by or affiliated with the Prism Launcher project.<br />
  <i>"Mono" is Spanish for monkey.</i>
</p>

## Installation

- Builds are published on the [Releases page](https://github.com/jurek-zsl/MonoLauncher/releases).
- Latest build status can be found in the [GitHub Actions](https://github.com/jurek-zsl/MonoLauncher/actions) tab.

## Support

Please report bugs and feature requests on the [MonoLauncher issue tracker](https://github.com/jurek-zsl/MonoLauncher/issues), not to the Prism Launcher project.

## Building

MonoLauncher builds the same way as Prism Launcher; see Prism's [build instructions](https://prismlauncher.org/wiki/development/build-instructions).

## Credits

MonoLauncher is built on the work of the [Prism Launcher](https://prismlauncher.org), PolyMC and MultiMC contributors. The MonoLauncher logo is a derivative of the Prism Launcher logo.

## Forking/Redistributing/Custom builds policy (inherited from Prism Launcher)

You are free to fork, redistribute and provide custom builds as long as you follow the terms of the [license](LICENSE) (this is a legal responsibility), and if you made code changes rather than just packaging a custom build, please do the following as a basic courtesy:

- Make it clear that your fork is not Prism Launcher and is not endorsed by or affiliated with the Prism Launcher project (<https://prismlauncher.org>).
- Go through [CMakeLists.txt](CMakeLists.txt) and change Prism Launcher's API keys to your own or set them to empty strings (`""`) to disable them (this way the program will still compile but the functionality requiring those keys will be disabled).

If you have any questions or want any clarification on the above conditions please make an issue and ask us.

If you are just building Prism Launcher for your distribution, please make sure to set the `Launcher_BUILD_PLATFORM` to a slug representing your distribution. Examples are `archlinux`, `fedora` and `nixpkgs`.

Note that if you build this software without removing the provided API keys in [CMakeLists.txt](CMakeLists.txt) you are accepting the following terms and conditions:

- [Microsoft Identity Platform Terms of Use](https://docs.microsoft.com/en-us/legal/microsoft-identity-platform/terms-of-use)
- [CurseForge 3rd Party API Terms and Conditions](https://support.curseforge.com/en/support/solutions/articles/9000207405-curse-forge-3rd-party-api-terms-and-conditions)

If you do not agree with these terms and conditions, then remove the associated API keys from the [CMakeLists.txt](CMakeLists.txt) file by setting them to an empty string (`""`).

## License

All launcher code is available under the GPL-3.0-only license.

The logo and related assets are under the CC BY-SA 4.0 license.
