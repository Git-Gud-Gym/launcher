# Playing offline

Git Gud Gym is for **offline play only**. The app refuses to attach while Easy Anti-Cheat (EAC) is running,
and it can start the game offline for you.

> [!WARNING]
> Modding a game online can get your account flagged. Don't try to work around the anti-cheat check. Use
> Git Gud Gym offline, at your own risk.

## The easy way: Launch game

Click **Launch game** in the app. The first time, it asks before creating `steam_appid.txt` in the game folder.
That file lets the game's exe start directly, offline, instead of relaunching through Steam with anti-cheat.
After that, **Launch game** starts the game without EAC every time.

## By hand (Elden Ring on Steam)

1. Open the game's `Game` folder, the one that contains `eldenring.exe`.
2. Create a text file named `steam_appid.txt` next to `eldenring.exe` containing only:

   ```text
   1245620
   ```

3. With Steam running, start `eldenring.exe` directly. Don't use Steam's **Play** button, which starts the
   game with anti-cheat.

> [!TIP]
> Windows hides file extensions by default, so a file you name `steam_appid.txt` in Explorer can end up as
> `steam_appid.txt.txt`. Turn on **View → File name extensions** to check.

## Playing online again

Delete `steam_appid.txt` from the game folder and start the game with Steam's **Play** button as usual.
Close Git Gud Gym first.
