# block tournaments

You can place a block tournament spawn tile (index 131) in the map. Then make sure ``sv_allow_block_tourna`` is set to ``1``.
A admin or a vote can use the command ``start_block_tournament`` to start a tournament.
Then all players have to opt in using the **/join** chat command.

If you want the player to be able to start tournaments without admin actions you can add a vote to your autoexec_server.cfg like this.

```
add_vote "Start Tournament" start_block_tournament
```

![tourna join](https://raw.githubusercontent.com/DDNetPP/cdn/refs/heads/master/tourna_join.png)
![tourna countdown](https://raw.githubusercontent.com/DDNetPP/cdn/refs/heads/master/tourna_countdown.png)
![tourna win](https://raw.githubusercontent.com/DDNetPP/cdn/refs/heads/master/tourna_win.png)

