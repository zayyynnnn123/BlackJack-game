# Blackjack (C++ / SFML)

A graphical Blackjack game built in C++ using [SFML 3](https://www.sfml-dev.org/) for rendering, with card artwork from [Kenney's Playing Cards Pack](https://kenney.nl/assets/playing-cards-pack) (CC0).

![status](https://img.shields.io/badge/status-in%20progress-yellow)

## Features

- Full Blackjack rules: dealer hits until 17, Ace counted as 1 or 11 automatically, blackjack pays 3:2
- Click-based Hit / Stand / New Round buttons — no console input required
- Card hands rendered as real sprites using Kenney's card artwork
- Dealer's first card stays hidden until the round ends
- Betting and running money total across rounds

## Built With

- **C++17**
- **SFML 3.0.2** — windowing, graphics, input
- **MSYS2 / MinGW-w64 (UCRT64)** — compiler toolchain

## Project Structure

```
.
├── blackjack_sfml.cpp        # All game logic + rendering (Card, Deck, Player, Game, Renderer)
├── assets/
│   ├── PNG/
│   │   └── Cards (large)/    # Card face images (Kenney asset pack)
│   └── font.ttf               # Font used for in-game text
└── .vscode/
    └── tasks.json             # VS Code build task (g++ command with SFML linked)
```

## Prerequisites

You'll need MSYS2 with the UCRT64 toolchain and SFML installed:

1. Install [MSYS2](https://www.msys2.org/)
2. Open the **MSYS2 UCRT64** terminal (not the default MSYS2 terminal) and run:
   ```bash
   pacman -S mingw-w64-ucrt-x86_64-sfml
   ```
3. Add `C:\msys64\ucrt64\bin` to your **User** PATH environment variable, so `g++` and SFML's DLLs are reachable from any terminal (including inside VS Code).
4. Restart your terminal / VS Code after updating PATH.

Verify the setup:
```powershell
g++ --version
```

## Building

From the project root:

```powershell
g++ -std=c++17 blackjack_sfml.cpp -IC:\msys64\ucrt64\include -LC:\msys64\ucrt64\lib -lsfml-graphics -lsfml-window -lsfml-system -o blackjack_sfml.exe
```

Or, in VS Code, just press **Ctrl+Shift+B** to run the build task defined in `.vscode/tasks.json`.

## Running

```powershell
.\blackjack_sfml.exe
```

A window should open showing the card table. Click **Hit** to draw a card, **Stand** to end your turn and let the dealer play, and **New Round** to deal again.

## Notes

- The card asset path in `blackjack_sfml.cpp` (`TextureManager::loadAll(...)`) must exactly match your local folder name and casing, including any spaces (e.g. `Cards (large)` vs `Cards(large)`) — SFML will print the exact path it tried and failed to find if this is wrong.
- `font.ttf` can be any `.ttf` font file; nothing about it is special beyond that.

## Credits

- Card artwork: [Kenney.nl](https://kenney.nl/) — Playing Cards Pack (CC0, free for any use)
- Built as a learning project for C++ classes, inheritance/composition design, and basic 2D game rendering with SFML.

## License

This project's source code is free to use, modify, and learn from. Kenney's assets retain their CC0 license (public domain — no attribution legally required, though it's appreciated).
