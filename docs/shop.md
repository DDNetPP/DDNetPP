# shop

ddnet++ has a shop where users can buy items using the in game currency.
The shop is a physical place in the map (game tile index 161) and can also be accessed with the chat command /shop. If ``sv_autoconnect_bots 1`` is set a bot will spawn in the shop which can be hammered to open the shop menu.

![slash shop](https://raw.githubusercontent.com/DDNetPP/cdn/refs/heads/master/chat_cmd_shop.png)

![shop list](https://raw.githubusercontent.com/DDNetPP/cdn/refs/heads/master/chat_shop_list.png)

The chat command /shop can be used anywhere in the map. It then lists all shop items.


Admins can use the following commands to customize the shop
```
set_shop_item_price [item] [price]
set_shop_item_description [item] [description]
set_shop_item_level [item] [level]
activate_shop_item [item]
deactivate_shop_item [item]
deactivate_all_shop_items
activate_all_shop_items
```

Checkout this video to see it in action.

[![shop config](https://i1.ytimg.com/vi/SS2x0JVtHZA/maxresdefault.jpg)](https://youtu.be/SS2x0JVtHZA)
