TSharedRef<SWidget> SMarketMenu::PricesPage()
{
    auto G = [this] { return Game.Get(); };
    auto Current = [G] { return G() && G()->Products.IsValidIndex(G()->MenuProduct) ? G()->MenuProduct : 0; };

    TSharedRef<SScrollBox> List = SNew(SScrollBox);
    const int32 Count = G() ? G()->Products.Num() : 0;
    for (int32 I = 0; I < Count; ++I)
    {
        List->AddSlot().Padding(0.f, 3.f)
        [
            SNew(SBox)
            .Visibility_Lambda([this, G, I] { return G() && (PriceCategory.IsEmpty() || G()->Products[I].Category == PriceCategory) ? EVisibility::Visible : EVisibility::Collapsed; })
            [
                SNew(SButton).ButtonStyle(&RowStyle).IsFocusable(false).ContentPadding(FMargin(12.f, 8.f))
                .ButtonColorAndOpacity_Lambda([this, Current, I] { return FSlateColor(Color(Current() == I ? ERole::Inset : ERole::Panel)); })
                .OnClicked_Lambda([G, I] { if (AMarketGameMode* M = G()) M->MenuProduct = I; return FReply::Handled(); })
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
                    [ Badge(FString(), MarketMenuUi::Initials(G()->ProductName(I)), FLinearColor(G()->Products[I].Color), 34.f) ]
                    + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
                    [
                        SNew(SVerticalBox)
                        + SVerticalBox::Slot().AutoHeight()[ Label([G, I] { return G() ? G()->ProductName(I) : FString(); }, 11, ERole::Text, true) ]
                        + SVerticalBox::Slot().AutoHeight()[ Label([G, I] { return G() ? MarketMenuUi::Title(G()->Products[I].Category) : FString(); }, 9, ERole::Muted) ]
                    ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f)
                    [
                        SNew(SVerticalBox)
                        + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)[ Label([G, I] { return G() ? MarketMenuUi::Tl(G()->State.Stock[I].Price) : FString(); }, 12, ERole::Text, true) ]
                        + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)
                        [
                            Label([G, I]
                            {
                                int64 Price = 0;
                                return G() && MarketMenuUi::CheapestRival(*G(), I, Price) != INDEX_NONE ? FString::Printf(TEXT("rakip en ucuz %s"), *MarketMenuUi::Tl(Price)) : FString(TEXT("rakiplerde yok"));
                            }, 9, ERole::Muted)
                        ]
                    ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                    [
                        SNew(SBox).WidthOverride(64.f).HAlign(HAlign_Right)
                        [
                            LabelBy([G, I]
                            {
                                int64 Price = 0;
                                if (!G() || MarketMenuUi::CheapestRival(*G(), I, Price) == INDEX_NONE) return FString(TEXT("tek biz"));
                                const int64 Ours = G()->State.Stock[I].Price;
                                return Ours < Price ? FString(TEXT("ucuz")) : Ours > Price ? FString(TEXT("pahal\u0131")) : FString(TEXT("ayn\u0131"));
                            }, 10, [G, I]
                            {
                                int64 Price = 0;
                                if (!G() || MarketMenuUi::CheapestRival(*G(), I, Price) == INDEX_NONE) return ERole::Good;
                                const int64 Ours = G()->State.Stock[I].Price;
                                return Ours < Price ? ERole::Good : Ours > Price ? ERole::Bad : ERole::Muted;
                            }, true)
                        ]
                    ]
                ]
            ]
        ];
    }

    TSharedRef<SVerticalBox> RivalRows = SNew(SVerticalBox);
    for (int32 Rival = 0; Rival < 3; ++Rival)
    {
        RivalRows->AddSlot().AutoHeight().Padding(0.f, 3.f)
        [
            SNew(SBorder).BorderImage(&SmallBrush).BorderBackgroundColor(Col(ERole::Inset)).Padding(FMargin(12.f, 8.f))
            .Visibility_Lambda([G, Rival] { return G() && Rival < MarketRivals::RivalCount(G()->State.Day) ? EVisibility::Visible : EVisibility::Collapsed; })
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 12.f, 0.f)
                [ Badge(MarketRivals::RivalLogoKey(Rival), MarketMenuUi::Initials(MarketRivals::RivalName(Rival)), MarketMenuUi::RivalColor(Rival), 34.f) ]
                + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()[ Fixed(MarketRivals::RivalName(Rival), 12, ERole::Text, true) ]
                    + SVerticalBox::Slot().AutoHeight()
                    [
                        Label([G, Current, Rival]
                        {
                            if (!G()) return FString();
                            bool bEmpty = false;
                            const float Factor = MarketRivals::RivalFactor(G()->State.Day, G()->State.RivalSeed, G()->RivalAisles, G()->Products[Current()].Category, Rival, &bEmpty);
                            const FString Kind = MarketRivals::RivalFormat(Rival);
                            if (bEmpty) return Kind + TEXT(" \u00b7 bu reyon bo\u015f");
                            if (Factor < 1.f) return Kind + FString::Printf(TEXT(" \u00b7 kampanyada %%%d"), FMath::RoundToInt32((1.f - Factor) * 100.f));
                            if (Factor > 1.f) return Kind + FString::Printf(TEXT(" \u00b7 zam %%%d"), FMath::RoundToInt32((Factor - 1.f) * 100.f));
                            return Kind;
                        }, 9, ERole::Muted)
                    ]
                ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f)
                [
                    Label([G, Current, Rival]
                    {
                        int64 Price = 0;
                        return G() && MarketMenuUi::RivalShelfPrice(*G(), Current(), Rival, Price) ? MarketMenuUi::Tl(Price) : FString(TEXT("Rafta yok"));
                    }, 14, ERole::Text, true)
                ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                [
                    SNew(SBox).WidthOverride(70.f).HAlign(HAlign_Right)
                    [
                        LabelBy([G, Current, Rival]
                        {
                            int64 Price = 0;
                            if (!G() || !MarketMenuUi::RivalShelfPrice(*G(), Current(), Rival, Price) || Price <= 0) return FString();
                            const int32 Diff = FMath::RoundToInt32((static_cast<double>(G()->State.Stock[Current()].Price) / Price - 1.0) * 100.0);
                            return Diff == 0 ? FString(TEXT("ayn\u0131")) : FString::Printf(TEXT("biz %s%%%d"), Diff > 0 ? TEXT("+") : TEXT("-"), FMath::Abs(Diff));
                        }, 10, [G, Current, Rival]
                        {
                            int64 Price = 0;
                            if (!G() || !MarketMenuUi::RivalShelfPrice(*G(), Current(), Rival, Price)) return ERole::Muted;
                            return G()->State.Stock[Current()].Price > Price ? ERole::Bad : ERole::Good;
                        }, true)
                    ]
                ]
            ]
        ];
    }

    TSharedRef<SWidget> Detail = Card(
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()[ Label([G, Current] { return G() ? G()->ProductName(Current()) : FString(); }, 20, ERole::Text, true) ]
                + SVerticalBox::Slot().AutoHeight()
                [ Label([G, Current] { return G() ? FString::Printf(TEXT("%s \u00b7 %s"), *MarketMenuUi::Title(G()->Products[Current()].Category), *G()->Products[Current()].Brand) : FString(); }, 10, ERole::Muted) ]
            ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 16.f, 0.f, 0.f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()[ Fixed(TEXT("B\u0130Z\u0130M F\u0130YAT"), 9, ERole::Muted, true) ]
                + SVerticalBox::Slot().AutoHeight()[ Label([G, Current] { return G() ? MarketMenuUi::Tl(G()->State.Stock[Current()].Price) : FString(); }, 28, ERole::Text, true) ]
            ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(18.f, 0.f, 6.f, 0.f)
            [ Button([] { return FString(TEXT("-")); }, [this, Current] { Do(TEXT("PriceDown"), Current()); }) ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
            [ Button([] { return FString(TEXT("+")); }, [this, Current] { Do(TEXT("PriceUp"), Current()); }) ]
            + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center).HAlign(HAlign_Right)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)[ Fixed(TEXT("ALAN M\u00dc\u015eTER\u0130"), 9, ERole::Muted, true) ]
                + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)
                [ LabelBy([G, Current] { return G() ? FString::Printf(TEXT("~%%%d"), FMath::RoundToInt32(MarketMenuUi::BuyChanceOf(*G(), Current()) * 100.0)) : FString(); }, 24,
                    [G, Current] { const double Chance = G() ? MarketMenuUi::BuyChanceOf(*G(), Current()) : 1.0; return Chance >= 0.6 ? ERole::Good : Chance >= 0.35 ? ERole::Warn : ERole::Bad; }, true) ]
            ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
        [
            Label([G, Current]
            {
                if (!G()) return FString();
                const FMarketProduct& P = G()->Products[Current()];
                const int64 Ours = G()->State.Stock[Current()].Price;
                const int32 Margin = Ours > 0 ? FMath::RoundToInt32(static_cast<double>(Ours - P.Cost) / Ours * 100.0) : 0;
                return FString::Printf(TEXT("Al\u0131\u015f %s \u00b7 liste fiyat\u0131 %s \u00b7 k\u00e2r marj\u0131 %%%d"), *MarketMenuUi::Tl(P.Cost), *MarketMenuUi::Tl(P.BasePrice), Margin);
            }, 10, ERole::Muted)
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 18.f, 0.f, 6.f)[ Fixed(TEXT("RAK\u0130P F\u0130YATLARI \u00b7 L\u00dcLEBURGAZ"), 9, ERole::Muted, true) ]
        + SVerticalBox::Slot().AutoHeight()[ RivalRows ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
        [
            Label([G, Current]
            {
                if (!G()) return FString();
                const int64 Theirs = MarketDemand::RivalPrice(G()->Products[Current()], G()->RivalPriceFactor(Current()));
                return FString::Printf(TEXT("M\u00fc\u015fterinin akl\u0131ndaki rakip fiyat\u0131: %s. Pahal\u0131 bulan m\u00fc\u015fteri \u00fcr\u00fcn\u00fc almaz; sad\u0131k m\u00fc\u015fteri biraz daha ho\u015fg\u00f6r\u00fcl\u00fcd\u00fcr."), *MarketMenuUi::Tl(Theirs));
            }, 10, ERole::Muted, false, true)
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
        [ Label([G, Current] { return G() ? G()->OrderAdvice(Current()) : FString(); }, 10, ERole::Muted, false, true) ]
        // G-064 promotions for the selected product / its aisle; running ones and the wholesaler's offer.
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 18.f, 0.f, 6.f)[ Fixed(TEXT("KAMPANYA"), 9, ERole::Muted, true) ]
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(SWrapBox).UseAllottedSize(true).InnerSlotPadding(FVector2D(6.f, 6.f))
            + SWrapBox::Slot()[ Button([] { return FString(TEXT("Reyonda %10")); }, [this, Current] { Manage(TEXT("Discount10"), Current()); }) ]
            + SWrapBox::Slot()[ Button([] { return FString(TEXT("Reyonda %20")); }, [this, Current] { Manage(TEXT("Discount20"), Current()); }) ]
            + SWrapBox::Slot()[ Button([] { return FString(TEXT("3 al 2 \u00f6de")); }, [this, Current] { Manage(TEXT("MultiBuy"), Current()); }) ]
            + SWrapBox::Slot()[ Button([] { return FString(TEXT("Gondol ba\u015f\u0131na koy")); }, [this, Current] { Manage(TEXT("Endcap"), Current()); }) ]
            + SWrapBox::Slot()[ Button([] { return FString(TEXT("Bro\u015f\u00fcr da\u011f\u0131t")); }, [this] { Manage(TEXT("Flyer"), INDEX_NONE); }) ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
        [
            Label([G]
            {
                if (!G()) return FString();
                TArray<FString> Lines;
                for (const FMarketPromotion* P : MarketPromotions::Active(G()->State))
                    Lines.Add(FString::Printf(TEXT("\u2022 %s \u00b7 %d. g\u00fcne kadar"), *MarketPromotions::Describe(*P, G()->Products), P->EndDay));
                if (G()->State.Offer.Product != INDEX_NONE && G()->State.Offer.EndDay >= G()->State.Day)
                    Lines.Add(TEXT("Toptanc\u0131 teklifi bekliyor: ") + MarketPromotions::Describe(G()->State.Offer, G()->Products));
                return Lines.Num() > 0 ? FString::Join(Lines, TEXT("\n")) : FString(TEXT("Y\u00fcr\u00fcyen kampanya yok."));
            }, 10, ERole::Text, false, true)
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 6.f, 0.f)
            [ Button([] { return FString(TEXT("Teklifi kabul et")); }, [this] { Manage(TEXT("AcceptOffer"), INDEX_NONE); }, true,
                [G] { return G() && G()->State.Offer.Product != INDEX_NONE && G()->State.Offer.EndDay >= G()->State.Day; }) ]
            + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 6.f, 0.f)
            [ Button([] { return FString(TEXT("Geri \u00e7evir")); }, [this] { Manage(TEXT("DeclineOffer"), INDEX_NONE); }, false,
                [G] { return G() && G()->State.Offer.Product != INDEX_NONE && G()->State.Offer.EndDay >= G()->State.Day; }) ]
            + SHorizontalBox::Slot().AutoWidth()
            [ Button([] { return FString(TEXT("Son kampanyay\u0131 durdur")); }, [this, G]
                {
                    if (!G()) return;
                    for (int32 I = G()->State.Promotions.Num() - 1; I >= 0; --I)
                        if (MarketPromotions::IsActive(G()->State.Promotions[I], G()->State.Day)) { Manage(TEXT("StopPromotion"), I); return; }
                }, false, [G] { return G() && MarketPromotions::Active(G()->State).Num() > 0; }) ]
        ],
        ERole::Panel, FMargin(22.f, 20.f));

    return SNew(SHorizontalBox)
        + SHorizontalBox::Slot().FillWidth(0.9f).Padding(0.f, 0.f, 16.f, 0.f)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)[ CategoryChips(&PriceCategory) ]
            + SVerticalBox::Slot().FillHeight(1.f)[ List ]
        ]
        + SHorizontalBox::Slot().FillWidth(1.f)
        [ SNew(SScrollBox) + SScrollBox::Slot()[ Detail ] ];
}

TSharedRef<SWidget> SMarketMenu::RivalsPage()
{
    auto G = [this] { return Game.Get(); };
    TSharedRef<SHorizontalBox> Cards = SNew(SHorizontalBox);
    for (int32 Rival = 0; Rival < 3; ++Rival)
    {
        Cards->AddSlot().FillWidth(1.f).Padding(0.f, 0.f, Rival < 2 ? 12.f : 0.f, 0.f)
        [
            SNew(SBox)
            .Visibility_Lambda([G, Rival] { return G() && Rival < MarketRivals::RivalCount(G()->State.Day) ? EVisibility::Visible : EVisibility::Hidden; })
            [
                Card(SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()
                    [
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 12.f, 0.f)
                        [ Badge(MarketRivals::RivalLogoKey(Rival), MarketMenuUi::Initials(MarketRivals::RivalName(Rival)), MarketMenuUi::RivalColor(Rival), 44.f) ]
                        + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
                        [
                            SNew(SVerticalBox)
                            + SVerticalBox::Slot().AutoHeight()[ Fixed(MarketRivals::RivalName(Rival), 16, ERole::Text, true) ]
                            + SVerticalBox::Slot().AutoHeight()[ Fixed(MarketRivals::RivalFormat(Rival), 10, ERole::Muted) ]
                        ]
                    ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 16.f, 0.f, 6.f)[ Fixed(TEXT("BUG\u00dcN"), 9, ERole::Muted, true) ]
                    + SVerticalBox::Slot().AutoHeight()[ Label([G, Rival] { return G() ? MarketMenuUi::RivalsToday(*G(), Rival) : FString(); }, 11, ERole::Text, false, true) ])
            ]
        ];
    }
    return SNew(SScrollBox)
    + SScrollBox::Slot()
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()
        [
            Card(SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()[ Fixed(TEXT("YEREL PAZAR \u00b7 L\u00dcLEBURGAZ"), 9, ERole::Muted, true) ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 8.f)
                    [ Label([G] { return G() ? FString::Printf(TEXT("Mahalle m\u00fc\u015fterilerinin %%%.0f'i bizden al\u0131\u015fveri\u015f yap\u0131yor"), G()->State.MarketShare) : FString(); }, 16, ERole::Text, true) ]
                    + SVerticalBox::Slot().AutoHeight()[ Bar([G] { return G() ? G()->State.MarketShare / 100.f : 0.f; }, ERole::Accent) ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
                    [ Fixed(TEXT("Pay her g\u00fcn sonunda m\u00fc\u015fteri memnuniyetiyle de\u011fi\u015fir: rafta bulma, fiyat ve bekleme."), 10, ERole::Muted) ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
                    [ Label([G]
                    {
                        if (!G()) return FString();
                        TArray<FString> Lines;
                        for (int32 C = 0; C < static_cast<int32>(MarketCompetitors::ECompany::Count); ++C)
                            Lines.Add(TEXT("\u2022 ") + MarketCompetitors::Describe(G()->State, static_cast<MarketCompetitors::ECompany>(C)));
                        return FString::Join(Lines, TEXT("\n"));
                    }, 10, ERole::Text, false, true) ]
                ])
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)[ Cards ]
        + SVerticalBox::Slot().AutoHeight().Padding(4.f, 14.f, 0.f, 0.f)
        [ Fixed(TEXT("Ulusal ve uluslararas\u0131 pazar paylar\u0131 \u015fube sistemiyle gelecek. Logo i\u00e7in: Content/Brands/<bim|migros|a101>/logo.png"), 10, ERole::Muted) ]
    ];
}

TSharedRef<SWidget> SMarketMenu::StaffPage()
{
    // G-060: people are persons (MarketStaff). Rows are fixed slots shown while the roster/pool has that many
    // entries; every button reads the person's id at click time, so a changed roster never hits the wrong person.
    auto G = [this] { return Game.Get(); };
    auto Act = [this](FName Action, int32 Id) { if (AMarketGameMode* Mode = Game.Get()) Mode->StaffCommand(Action, Id); };
    auto PersonAt = [G](int32 Slot) -> const FMarketEmployee* { return G() && G()->State.Staff.IsValidIndex(Slot) ? &G()->State.Staff[Slot] : nullptr; };
    auto CandidateAt = [G](int32 Slot) -> const FMarketEmployee* { return G() && G()->State.Candidates.IsValidIndex(Slot) ? &G()->State.Candidates[Slot] : nullptr; };
    auto RoleAt = [PersonAt](int32 Slot) { const FMarketEmployee* E = PersonAt(Slot); return E ? MarketStaff::RoleOf(*E) : MarketStaff::ERole::Accountant; };

    TSharedRef<SVerticalBox> People = SNew(SVerticalBox);
    for (int32 Slot = 0; Slot < 8; ++Slot)
    {
        People->AddSlot().AutoHeight().Padding(0.f, 3.f)
        [
            SNew(SBox).Visibility_Lambda([PersonAt, Slot] { return PersonAt(Slot) ? EVisibility::Visible : EVisibility::Collapsed; })
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center).Padding(0.f, 0.f, 8.f, 0.f)
                [ Label([G, PersonAt, Slot] { const FMarketEmployee* E = PersonAt(Slot); return E ? MarketStaff::DescribeEmployee(G()->State, *E) : FString(); }, 11, ERole::Text, false, true) ]
                + SHorizontalBox::Slot().AutoWidth().Padding(4.f, 0.f)
                [ Button([] { return FString(TEXT("Zam %10")); }, [Act, PersonAt, Slot] { if (const FMarketEmployee* E = PersonAt(Slot)) Act(TEXT("Raise"), E->Id); }, false,
                    [RoleAt, PersonAt, Slot] { return PersonAt(Slot) && RoleAt(Slot) != MarketStaff::ERole::Accountant; }) ]
                + SHorizontalBox::Slot().AutoWidth().Padding(4.f, 0.f)
                [ Button([] { return FString(TEXT("\u0130zin ver")); }, [Act, PersonAt, Slot] { if (const FMarketEmployee* E = PersonAt(Slot)) Act(TEXT("DayOff"), E->Id); }, false,
                    [RoleAt, PersonAt, Slot] { return PersonAt(Slot) && RoleAt(Slot) != MarketStaff::ERole::Accountant; }) ]
                + SHorizontalBox::Slot().AutoWidth().Padding(4.f, 0.f)
                [ Button([] { return FString(TEXT("Uyar")); }, [Act, PersonAt, Slot] { if (const FMarketEmployee* E = PersonAt(Slot)) Act(TEXT("Warn"), E->Id); }, false,
                    [RoleAt, PersonAt, Slot] { return PersonAt(Slot) && RoleAt(Slot) == MarketStaff::ERole::Cashier; }) ]
                + SHorizontalBox::Slot().AutoWidth().Padding(4.f, 0.f)
                [ Button([RoleAt, Slot] { return FString(RoleAt(Slot) == MarketStaff::ERole::Accountant ? TEXT("S\u00f6zle\u015fmeyi bitir") : TEXT("\u00c7\u0131kar")); },
                    [Act, PersonAt, Slot] { if (const FMarketEmployee* E = PersonAt(Slot)) Act(TEXT("Fire"), E->Id); }, false, [PersonAt, Slot] { return PersonAt(Slot) != nullptr; }) ]
            ]
        ];
    }

    TSharedRef<SVerticalBox> Pool = SNew(SVerticalBox);
    for (int32 Slot = 0; Slot < MarketStaff::HrPoolSize; ++Slot)
    {
        Pool->AddSlot().AutoHeight().Padding(0.f, 3.f)
        [
            SNew(SBox).Visibility_Lambda([CandidateAt, Slot] { return CandidateAt(Slot) ? EVisibility::Visible : EVisibility::Collapsed; })
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center).Padding(0.f, 0.f, 8.f, 0.f)
                [ Label([G, CandidateAt, Slot] { const FMarketEmployee* C = CandidateAt(Slot); return C ? MarketStaff::DescribeCandidate(G()->State, *C) : FString(); }, 11, ERole::Text, false, true) ]
                + SHorizontalBox::Slot().AutoWidth().Padding(4.f, 0.f)
                [ Button([] { return FString(TEXT("\u0130\u015fe al")); }, [Act, CandidateAt, Slot] { if (const FMarketEmployee* C = CandidateAt(Slot)) Act(TEXT("HireCandidate"), C->Id); }, true,
                    [CandidateAt, Slot] { return CandidateAt(Slot) != nullptr; }) ]
            ]
        ];
    }

    return SNew(SScrollBox)
    + SScrollBox::Slot()
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 12.f, 0.f)
            [
                Card(SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()[ Fixed(TEXT("EK\u0130P"), 9, ERole::Muted, true) ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 4.f)
                    [ Label([G] { return G() ? FString::Printf(TEXT("%d ki\u015fi \u00b7 g\u00fcnl\u00fck %s"), G()->State.Staff.Num(), *MarketMenuUi::Tl(G()->State.DailyPayroll())) : FString(); }, 18, ERole::Text, true) ]
                    + SVerticalBox::Slot().AutoHeight()
                    [ Label([G] { return G() ? FString::Printf(TEXT("Bug\u00fcn kasada: %s \u00b7 reyonda %d g\u00f6revli"),
                        G()->State.bCashier ? TEXT("kasiyer") : TEXT("sen (E)"), G()->State.Stockers) : FString(); }, 10, ERole::Muted, false, true) ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
                    [ Label([G] { return G() ? G()->WorkerSummary() : FString(); }, 10, ERole::Muted, false, true) ])
            ]
            + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 12.f, 0.f)
            [
                Card(SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()[ Fixed(TEXT("MAL\u0130 M\u00dc\u015eAV\u0130R \u00b7 VERG\u0130"), 9, ERole::Muted, true) ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 4.f)
                    [ LabelBy([G] { return G() && G()->State.Books.TaxDue > 0 ? FString::Printf(TEXT("%s \u00b7 son g\u00fcn %d"), *MarketMenuUi::Tl(G()->State.Books.TaxDue), G()->State.Books.TaxDueDay)
                        : FString(TEXT("\u00d6denecek vergi yok")); }, 18, [G] { return G() && G()->State.Books.TaxDue > 0 ? ERole::Warn : ERole::Good; }, true) ]
                    + SVerticalBox::Slot().AutoHeight()
                    [ Label([G] { return G() && MarketStaff::HasAccountant(G()->State)
                        ? FString(TEXT("Necati Bey defterleri tutuyor: haftal\u0131k vergiyi zaman\u0131nda \u00f6der, kasa farklar\u0131n\u0131 izler."))
                        : FString(TEXT("Vergi her 7. g\u00fcn\u00fcn sonunda \u00e7\u0131kar; 3 g\u00fcn i\u00e7inde \u00f6denmezse ceza i\u015fler. Defter tutulmazsa inceleme gelebilir.")); }, 10, ERole::Muted, false, true) ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f).HAlign(HAlign_Left)
                    [
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
                        [ Button([] { return FString(TEXT("Vergiyi \u00f6de")); }, [Act] { Act(TEXT("PayTax"), INDEX_NONE); }, true, [G] { return G() && G()->State.Books.TaxDue > 0; }) ]
                        + SHorizontalBox::Slot().AutoWidth()
                        [ Button([] { return FString::Printf(TEXT("M\u00fc\u015favirle anla\u015f (%s/g\u00fcn)"), *MarketMenuUi::Tl(MarketStaff::AccountantDailyFee)); },
                            [Act] { Act(TEXT("HireAccountant"), INDEX_NONE); }, false, [G] { return G() && !MarketStaff::HasAccountant(G()->State); }) ]
                    ])
            ]
            + SHorizontalBox::Slot().FillWidth(1.f)
            [
                Card(SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()[ Fixed(TEXT("\u0130NSAN KAYNAKLARI"), 9, ERole::Muted, true) ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 4.f)
                    [ Label([G]
                    {
                        if (!G()) return FString();
                        if (MarketStaff::HasHr(G()->State)) return FString(TEXT("\u0130K m\u00fcd\u00fcr\u00fc \u00e7al\u0131\u015f\u0131yor"));
                        return MarketStaff::HrUnlocked(G()->State) ? FString(TEXT("Aday listesinde \u0130K m\u00fcd\u00fcr\u00fc var")) : FString::Printf(TEXT("%d \u00e7al\u0131\u015fandan sonra"), MarketStaff::HrUnlockStaff);
                    }, 18, ERole::Text, true) ]
                    + SVerticalBox::Slot().AutoHeight()
                    [ Fixed(TEXT("Her g\u00fcn en mutsuz ki\u015fiyle konu\u015fur, yorgunlara izin ayarlar, ayr\u0131lan\u0131n yerine aday bulur, \u00fccret pazarl\u0131\u011f\u0131 yapar ve adaylar\u0131n ger\u00e7ek de\u011ferlerini g\u00f6sterir."), 10, ERole::Muted) ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f).HAlign(HAlign_Left)
                    [ Button([G] { return FString(G() && G()->State.bHrAutoReplace ? TEXT("Ayr\u0131lan\u0131n yerine al: a\u00e7\u0131k") : TEXT("Ayr\u0131lan\u0131n yerine al: kapal\u0131")); },
                        [Act] { Act(TEXT("HrAutoReplace"), INDEX_NONE); }, false, [G] { return G() && MarketStaff::HasHr(G()->State); }) ])
            ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
        [
            Card(SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)[ Fixed(TEXT("\u00c7ALI\u015eANLAR"), 9, ERole::Muted, true) ]
                + SVerticalBox::Slot().AutoHeight()
                [
                    SNew(SBox).Visibility_Lambda([G] { return G() && G()->State.Staff.Num() == 0 ? EVisibility::Visible : EVisibility::Collapsed; })
                    [ Fixed(TEXT("Kimse yok. Kasay\u0131 ve raflar\u0131 sen yap\u0131yorsun."), 11, ERole::Muted) ]
                ]
                + SVerticalBox::Slot().AutoHeight()[ People ])
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
        [
            Card(SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)[ Fixed(TEXT("\u0130\u015e BA\u015eVURULARI"), 9, ERole::Muted, true) ]
                + SVerticalBox::Slot().AutoHeight()
                [ Label([G] { return G() ? FString::Printf(TEXT("\u0130\u015fe alma %s (\u0130K m\u00fcd\u00fcr\u00fc %s). Liste %s yenilenir; son yenileme %d. g\u00fcn."),
                    *MarketMenuUi::Tl(MarketStaff::HireCost), *MarketMenuUi::Tl(MarketStaff::HrHireCost),
                    MarketStaff::HasHr(G()->State) ? TEXT("3 g\u00fcnde bir") : TEXT("haftada bir"), G()->State.CandidatesDay) : FString(); }, 10, ERole::Muted, false, true) ]
                + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)[ Pool ])
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(4.f, 14.f, 0.f, 0.f)
        [ Fixed(TEXT("Moral \u00fccrete, yorgunlu\u011fa ve ilgiye g\u00f6re de\u011fi\u015fir. \u00dc\u00e7 g\u00fcn \u00e7ok mutsuz olan istifa dilek\u00e7esi verir ve iki g\u00fcn sonra ayr\u0131l\u0131r; zam veya izin fikrini de\u011fi\u015ftirebilir."), 10, ERole::Muted) ]
    ];
}

TSharedRef<SWidget> SMarketMenu::BranchesPage()
{
    // G-068: the family shop, the branches (simulated) and the districts where a new one can open.
    auto G = [this] { return Game.Get(); };
    TSharedRef<SVerticalBox> Shops = SNew(SVerticalBox);
    for (int32 Slot = 0; Slot < 8; ++Slot)
    {
        Shops->AddSlot().AutoHeight().Padding(0.f, 4.f)
        [
            SNew(SBorder).BorderImage(&SmallBrush).BorderBackgroundColor(Col(ERole::Inset)).Padding(FMargin(12.f, 8.f))
            .Visibility_Lambda([G, Slot] { return G() && G()->State.Branches.IsValidIndex(Slot) ? EVisibility::Visible : EVisibility::Collapsed; })
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
                [ Label([G, Slot] { return G() ? MarketBranches::Summary(G()->State, Slot, G()->Products) : FString(); }, 10, ERole::Text, false, true) ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
                [ Button([] { return FString(TEXT("Kapat")); }, [this, Slot] { Manage(TEXT("CloseBranch"), Slot); }, false,
                    [G, Slot] { return G() && G()->State.Branches.IsValidIndex(Slot) && G()->State.Branches[Slot].Stage != static_cast<uint8>(MarketBranches::EStage::Closed); }) ]
            ]
        ];
    }
    TSharedRef<SVerticalBox> Districts = SNew(SVerticalBox);
    for (int32 D = 1; D < static_cast<int32>(MarketBranches::EDistrict::Count); ++D)
    {
        const MarketBranches::EDistrict District = static_cast<MarketBranches::EDistrict>(D);
        Districts->AddSlot().AutoHeight().Padding(0.f, 3.f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
            [ Label([G, District]
            {
                const MarketBranches::FDistrict& Info = MarketBranches::DistrictInfo(District);
                return FString::Printf(TEXT("%s \u00b7 %s \u00b7 kira %s/ay \u00b7 a\u00e7\u0131l\u0131\u015f %s"), Info.Name, Info.Note, *MarketMenuUi::Tl(Info.Rent),
                    G() ? *MarketMenuUi::Tl(MarketBranches::OpeningCost(G()->State, G()->Products, District, TEXT("mahalle"))) : TEXT(""));
            }, 10, ERole::Text, false, true) ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 4.f, 0.f)
            [ Button([] { return FString(TEXT("K\u00fc\u00e7\u00fck a\u00e7")); }, [this, D] { Manage(TEXT("OpenBranch"), D * 10 + 0); }) ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
            [ Button([] { return FString(TEXT("Mahalle marketi a\u00e7")); }, [this, D] { Manage(TEXT("OpenBranch"), D * 10 + 1); }, true) ]
        ];
    }
    // G-072: stores in other cities (aggregate) and what the company builds.
    TSharedRef<SWrapBox> CityButtons = SNew(SWrapBox).UseAllottedSize(true).InnerSlotPadding(FVector2D(6.f, 6.f));
    for (int32 City = 0; City < static_cast<int32>(MarketCompany::ECity::Count); ++City)
    {
        const MarketCompany::ECity Id = static_cast<MarketCompany::ECity>(City);
        CityButtons->AddSlot()
        [ Button([G, Id]
            {
                const FMarketCityStores* Row = G() ? MarketCompany::Find(G()->State, Id) : nullptr;
                return FString::Printf(TEXT("%s +1 (%d)"), MarketCompany::CityInfo(Id).Name, Row ? Row->Stores : 0);
            },
            [this, City] { Manage(TEXT("OpenStore"), City); }, false,
            [G, Id] { return G() && MarketCompany::ChapterOpen(G()->State, MarketCompany::CityInfo(Id).Chapter); }) ];
    }
    static const TCHAR* BuildNames[5] = { TEXT("B\u00f6lge deposu"), TEXT("Kamyon al"), TEXT("Merkezi sat\u0131n alma"), TEXT("\"Miras\" \u00f6zel markas\u0131"), TEXT("Karanl\u0131k ma\u011faza") };
    TSharedRef<SWrapBox> BuildButtons = SNew(SWrapBox).UseAllottedSize(true).InnerSlotPadding(FVector2D(6.f, 6.f));
    for (int32 What = 0; What < 5; ++What)
        BuildButtons->AddSlot()[ Button([What] { return FString(BuildNames[What]); }, [this, What] { Manage(TEXT("Build"), What); }, false,
            [G] { return G() && MarketCompany::ChapterOpen(G()->State, 4); }) ];
    return SNew(SScrollBox)
    + SScrollBox::Slot()
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.4f).Padding(0.f, 0.f, 12.f, 0.f)
            [
                Card(SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()[ Fixed(TEXT("\u015eUBELER\u0130N"), 9, ERole::Muted, true) ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
                    [
                        SNew(SBorder).BorderImage(&SmallBrush).BorderBackgroundColor(Col(ERole::Inset)).Padding(FMargin(12.f, 10.f))
                        [
                            SNew(SVerticalBox)
                            + SVerticalBox::Slot().AutoHeight()[ Fixed(TEXT("L\u00fcleburgaz \u00b7 \u0130stasyon"), 13, ERole::Text, true) ]
                            + SVerticalBox::Slot().AutoHeight()[ Fixed(TEXT("Babadan kalan mahalle marketi \u00b7 buradas\u0131n"), 10, ERole::Muted) ]
                        ]
                    ]
                    + SVerticalBox::Slot().AutoHeight()[ Shops ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
                    [ Label([G] { return G() && G()->State.Branches.Num() > 0 ? FString::Printf(TEXT("\u015eubelerin d\u00fcnk\u00fc toplam katk\u0131s\u0131 %s."), *MarketMenuUi::Tl(G()->State.LastBranchProfit)) : FString(TEXT("Hen\u00fcz \u015fube yok.")); }, 10, ERole::Muted) ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f).HAlign(HAlign_Left)
                    [ Button([] { return FString(TEXT("Cem'i \u015fube m\u00fcd\u00fcr\u00fc yap")); },
                        [this] { Manage(TEXT("Promote"), 900001); }, false,
                        [G] { return G() && MarketBranches::OpenCount(G()->State) > 0 && G()->State.Staff.ContainsByPredicate([](const FMarketEmployee& E) { return E.Id == 900001; }); }) ])
            ]
            + SHorizontalBox::Slot().FillWidth(1.f)
            [
                Card(SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)[ Fixed(TEXT("\u0130LK \u015eUBE \u0130\u00c7\u0130N"), 9, ERole::Muted, true) ]
                    + SVerticalBox::Slot().AutoHeight()[ GoalList() ])
            ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
        [
            Card(SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)[ Fixed(TEXT("L\u00dcLEBURGAZ SEMTLER\u0130 \u00b7 YEN\u0130 \u015eUBE"), 9, ERole::Muted, true) ]
                + SVerticalBox::Slot().AutoHeight()[ Districts ]
                + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
                [ Fixed(TEXT("A\u00e7\u0131l\u0131\u015f: depozito ve tadilat (5 g\u00fcn) \u2192 ruhsat (3 g\u00fcn; m\u00fc\u015favir yoksa 3 g\u00fcn daha) \u2192 i\u015fe al\u0131m \u2192 a\u00e7\u0131l\u0131\u015f sto\u011fu. Raflar otomatik planlan\u0131r; \u015fube her g\u00fcn ayn\u0131 kurallarla i\u015fler. \u00dc\u00e7\u00fcnc\u00fc \u015fube i\u00e7in \u0130K m\u00fcd\u00fcr\u00fc gerekir."), 10, ERole::Muted) ])
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
        [
            Card(SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)[ Fixed(TEXT("\u015e\u0130RKET \u00b7 TRAKYA, T\u00dcRK\u0130YE, SINIR \u00d6TES\u0130"), 9, ERole::Muted, true) ]
                + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
                [ Label([G] { return G() ? MarketCompany::Summary(G()->State) : FString(); }, 10, ERole::Text, false, true) ]
                + SVerticalBox::Slot().AutoHeight()[ CityButtons ]
                + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)[ BuildButtons ]
                + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
                [ Fixed(TEXT("\u015eehirler b\u00f6l\u00fcmle a\u00e7\u0131l\u0131r (Trakya 4, T\u00fcrkiye 5, s\u0131n\u0131r \u00f6tesi 6) ve \u0130K m\u00fcd\u00fcr\u00fc ile mali m\u00fc\u015favir ister. Uzak ma\u011fazalar depo ve kamyon olmadan marj kaybeder."), 10, ERole::Muted) ])
        ]
    ];
}
