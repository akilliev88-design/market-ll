"""Shared dimensions and fabrication data for G-088, centimetres except Blender output."""
EQUIPMENT = {
    'checkout_single': dict(name='CheckoutSingle', family='checkout', size=[110,250,110], levels=[], checkouts=1),
    'checkout_double': dict(name='CheckoutDouble', family='checkout', size=[220,250,110], levels=[], checkouts=2),
    'cooler_wall': dict(name='CoolerWall', family='cooler', size=[300,92,225], levels=[25,65,105,145,185]),
    'freezer_chest': dict(name='FreezerChest', family='freezer', size=[250,95,90], levels=[38]),
    'deli_counter': dict(name='DeliCounter', family='deli', size=[300,110,125], levels=[65]),
    'butcher_counter': dict(name='ButcherCounter', family='butcher', size=[300,110,125], levels=[65]),
    'fish_counter': dict(name='FishCounter', family='fish', size=[300,110,125], levels=[65]),
    'bakery_shelf': dict(name='BakeryShelf', family='bakery', size=[240,60,210], levels=[30,75,120,165]),
    'pallet_display': dict(name='PalletDisplay', family='pallet', size=[120,100,105], levels=[16]),
    'produce_small': dict(name='ProduceSmall', family='produce', size=[160,90,120], levels=[65]),
    'produce_large': dict(name='ProduceLarge', family='produce', size=[300,140,135], levels=[70]),
    'basket_area': dict(name='BasketArea', family='basket', size=[90,65,110], levels=[]),
    'cart_area': dict(name='CartArea', family='cart', size=[300,200,120], levels=[]),
    'tobacco_backbar': dict(name='TobaccoBackbar', family='tobacco', size=[120,35,210], levels=[40,80,120,160]),
    'customer_service': dict(name='CustomerService', family='service', size=[300,100,110], levels=[]),
    'home_display': dict(name='HomeDisplay', family='home', size=[240,65,210], levels=[]),
    'electronics_display': dict(name='ElectronicsDisplay', family='electronics', size=[240,65,210], levels=[]),
    'textile_display': dict(name='TextileDisplay', family='textile', size=[240,65,210], levels=[]),
}

STORES = {
    'mahalle_01': dict(name='Sıcak mahalle dükkânı', format='mahalle', theme='sicak_ahsap', footprint=[1200,1000], back=200, ceiling=300, cols=3, rows=3, xstep=270, ystep=160, first=-220, walls=4, cold=2, frozen=1, checkouts=1),
    'kucuk_01': dict(name='Aydınlık ucuzcu', format='kucuk', theme='aydinlik', footprint=[2000,1700], back=200, ceiling=360, cols=5, rows=6, xstep=300, ystep=155, first=-470, walls=4, cold=3, frozen=1, checkouts=2),
    'buyuk_01': dict(name='Reyon adalı süpermarket', format='buyuk', theme='dogal_yesil', footprint=[3600,2400], back=240, ceiling=420, cols=9, rows=9, xstep=290, ystep=150, first=-690, walls=8, cold=9, frozen=4, checkouts=5),
    'hiper_01': dict(name='Geniş koridorlu hipermarket', format='hiper', theme='soguk_beyaz', footprint=[8000,5000], back=500, ceiling=650, cols=17, rows=19, xstep=380, ystep=180, first=-1650, walls=16, cold=30, frozen=12, checkouts=18),
}
