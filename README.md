# Elimination III - HTML5 Port

A faithful HTML5/JavaScript port of **Elimination III**, a turn-based strategy board game originally developed by Justin Hoffman in 2000 as a Win32/DirectX application.

## Play Online

**[Play Elimination III](https://USERNAME.github.io/REPO_NAME/)**

> Replace the link above with your actual GitHub Pages URL after deployment.

## About the Game

Elimination III is a multiplayer board game where 2-4 players compete on a 16x16 grid. Each turn, players roll two dice and move their marbles across the board. The goal is to eliminate your opponents by shooting their marbles.

### Rules

- **Movement**: Roll two dice each turn. Use each die value to move one marble that many spaces in a cardinal direction (up/down/left/right).
- **Shooting**: If an enemy marble is in your path, a bullet is fired at the first enemy encountered, eliminating it and leaving a free marble pickup in its place.
- **Jumping**: Marbles can jump over any number of friendly or enemy marbles in their path (only the first enemy gets shot).
- **Free Marbles**: Landing on a free marble (F) grants you a new marble on your home edge.
- **Sinkholes**: Hidden traps scattered across the board. Landing on one destroys your marble and the sinkhole.
- **Winning**: The last player (or team) with marbles remaining wins.

### Features

- Original sprites, sounds, and animations faithfully recreated
- MIDI music playback using FM synthesis via Web Audio API
- Human vs Computer players with AI opponent
- Team play support
- Configurable sinkhole count and multi-game series
- Right-side menu bar with music/sound toggles

## Running Locally

Simply open `elim3/html/index.html` in a modern web browser. No build step or server required - the game runs entirely client-side.

## Credits

- **Original Game**: Justin Hoffman (2000)
- **HTML5 Port**: Assisted by Claude Code
