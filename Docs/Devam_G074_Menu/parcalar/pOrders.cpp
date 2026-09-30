TSharedRef<SWidget> SMarketMenu::OrdersPage()
{
    auto G = [this] { return Game.Get(); };
    auto Cell = [this](TFunction<FString()> Make, float Width, ERole Role = ERole::Text, bool bBold = false) -> TSharedRef<SWidget>
    {
        return SNew(SBox).WidthOverride(Width).HAlign(HAlign_Right)[ Label(Make, 11, Role, bBold) ];
    };
    auto Head = [this](const FString& Text, float Width) -> TSharedRef<SWidget>
    {
        return SNew(SBox).WidthOverride(Width).HAlign(HAlign_Right)[ Fixed(Text, 9, ERole::Muted, true) ];
    };

    TSharedRef<SScrollBox> Rows = SNew(SScrollBox);
    const int32 Count = G() ? G()->Products.Num() : 0;
    for (int32 I = 0; I < Count; ++I)
    {
        auto Item = [G, I]() -> const FMarketStock* { return G() && G()->State.Stock.IsValidIndex(I) ? &G()->State.Stock[I] : nullptr; };
        Rows->AddSlot().Padding(0.f, 3.f)
        [
            SNew(SBox)
            .Visibility_Lambda([this, G, I] { return G() && (OrderCategory.IsEmpty() || G()->Products[I].Category == OrderCategory) ? EVisibility::Visible : EVisibility::Collapsed; })
            [
                SNew(SBorder).BorderImage(&SmallBrush).BorderBackgroundColor(Col(ERole::Panel)).Padding(FMargin(12.f, 6.f))
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
                    [ Badge(FString(), MarketMenuUi::Initials(G()->ProductName(I)), FLinearColor(G()->Products[I].Color), 26.f) ]
                    + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
                    [
                        SNew(SVerticalBox)
                        + SVerticalBox::Slot().AutoHeight()[ Label([G, I] { return G() ? G()->ProductName(I) : FString(); }, 11, ERole::Text, true) ]
                        + SVerticalBox::Slot().AutoHeight()
                        [ Label([G, I] { return G() ? FString::Printf(TEXT("%s \u00b7 koli %d adet \u00b7 %s/adet"), *MarketMenuUi::Title(G()->Products[I].Category), MarketOrderAdvice::CaseUnits(G()->Products[I]), *MarketMenuUi::Tl(G()->Products[I].Cost)) : FString(); }, 9, ERole::Muted) ]
                    ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[ Cell([Item] { return Item() ? FString::Printf(TEXT("%d/%d"), Item()->Shelf, Item()->Capacity) : FString(); }, 70.f) ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[ Cell([Item] { return Item() ? FString::FromInt(Item()->Warehouse) : FString(); }, 56.f) ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[ Cell([Item] { return Item() ? FString::FromInt(Item()->Dock) : FString(); }, 56.f) ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[ Cell([Item] { return Item() ? FString::FromInt(Item()->Incoming) : FString(); }, 56.f) ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[ Cell([Item] { return Item() ? FString::FromInt(Item()->Yesterday.Sold) : FString(); }, 56.f) ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[ Cell([Item] { return Item() ? FString::FromInt(Item()->Yesterday.Empty) : FString(); }, 56.f, ERole::Warn) ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                    [ Cell([G, I] { if (!G() || !G()->Products.IsValidIndex(I)) return FString(); TArray<float> Scale; Scale.Init(1.f, G()->Products.Num()); Scale[I] = MarketDirector::OrderScale(G()->State, G()->Products[I]);
                        return FString::FromInt(MarketOrderAdvice::SuggestCases(G()->State, G()->Products, I, &Scale)); }, 60.f, ERole::Accent, true) ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(18.f, 0.f, 0.f, 0.f)
                    [
                        SNew(SBox).WidthOverride(150.f).HAlign(HAlign_Right)
                        [
                            SNew(SHorizontalBox)
                            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[ Button([] { return FString(TEXT("-")); }, [this, I] { Do(TEXT("RemoveOrder"), I); }, false,
                                [G, I] { return G() && G()->OrderDraftCases.IsValidIndex(I) && G()->OrderDraftCases[I] > 0; }) ]
                            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(6.f, 0.f)
                            [ SNew(SBox).WidthOverride(46.f).HAlign(HAlign_Center)
                                [ Label([G, I] { return G() && G()->OrderDraftCases.IsValidIndex(I) ? FString::Printf(TEXT("%d koli"), G()->OrderDraftCases[I]) : FString(TEXT("0 koli")); }, 11, ERole::Text, true) ] ]
                            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[ Button([] { return FString(TEXT("+")); }, [this, I] { Do(TEXT("Order"), I); }, true) ]
                        ]
                    ]
                ]
            ]
        ];
    }

    // G-063 wholesaler: who we buy from, terms, open bills and passing the monthly price rise on to the shelves.
    auto Act = [this](FName Action, int32 Arg) { if (AMarketGameMode* Mode = Game.Get()) Mode->StaffCommand(Action, Arg); };
    return SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
        [
            Card(
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
                [ Label([G] { return G() ? MarketSuppliers::Summary(G()->State) : FString(); }, 11, ERole::Text, false, true) ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
                [ Button([G] { return FString(G() && G()->State.Supplier == 0 ? TEXT("\u00d6zdemir'e ge\u00e7") : TEXT("Selim'e d\u00f6n")); },
                    [Act, G] { if (G()) Act(TEXT("Supplier"), G()->State.Supplier == 0 ? 1 : 0); }, false,
                    [G] { return G() && (G()->State.Supplier != 0 || MarketSuppliers::Available(G()->State, MarketSuppliers::ESupplier::Ozdemir)); }) ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
                [ Button([] { return FString(TEXT("Faturalar\u0131 \u00f6de")); }, [Act] { Act(TEXT("PayBills"), 0); }, false, [G] { return G() && MarketSuppliers::OpenBills(G()->State) > 0; }) ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
                [ Button([] { return FString(TEXT("Zamm\u0131 yans\u0131t")); }, [Act] { Act(TEXT("PassOnPriceRise"), 0); }, false, [G] { return G() && MarketSuppliers::PriceGap(G()->State) >= 0.005; }) ],
                ERole::Panel, FMargin(18.f, 10.f))
        ]
        + SVerticalBox::Slot().AutoHeight()[ CategoryChips(&OrderCategory) ]
        + SVerticalBox::Slot().AutoHeight().Padding(12.f, 14.f, 12.f, 4.f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f)[ Fixed(TEXT("\u00dcR\u00dcN"), 9, ERole::Muted, true) ]
            + SHorizontalBox::Slot().AutoWidth()[ Head(TEXT("RAF"), 70.f) ]
            + SHorizontalBox::Slot().AutoWidth()[ Head(TEXT("DEPO"), 56.f) ]
            + SHorizontalBox::Slot().AutoWidth()[ Head(TEXT("KABUL"), 56.f) ]
            + SHorizontalBox::Slot().AutoWidth()[ Head(TEXT("YOLDA"), 56.f) ]
            + SHorizontalBox::Slot().AutoWidth()[ Head(TEXT("D\u00dcN SAT."), 56.f) ]
            + SHorizontalBox::Slot().AutoWidth()[ Head(TEXT("BO\u015e RAF"), 56.f) ]
            + SHorizontalBox::Slot().AutoWidth()[ Head(TEXT("\u00d6NER\u0130"), 60.f) ]
            + SHorizontalBox::Slot().AutoWidth().Padding(18.f, 0.f, 0.f, 0.f)[ Head(TEXT("L\u0130STE"), 150.f) ]
        ]
        + SVerticalBox::Slot().FillHeight(1.f)[ Rows ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
        [
            Card(
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()
                    [ Label([G] { return G() ? FString::Printf(TEXT("Liste: %d koli \u00b7 %s"), G()->OrderDraftCaseCount(), *MarketMenuUi::Tl(G()->OrderDraftBill())) : FString(); }, 15, ERole::Text, true) ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f, 0.f, 0.f)
                    [
                        LabelBy([G]
                        {
                            if (!G()) return FString();
                            if (G()->bTestMode) return FString(TEXT("TEST MODU: + bedava ve an\u0131nda depoya getirir."));
                            if (G()->OrderDraftCaseCount() > 0 && G()->OrderDraftBill() < MarketOrderAdvice::MinimumOrder)
                                return FString::Printf(TEXT("Toptanc\u0131 en az %s sipari\u015fle gelir."), *MarketMenuUi::Tl(MarketOrderAdvice::MinimumOrder));
                            if (G()->OrderDraftBill() > G()->State.Cash) return FString(TEXT("Kasadaki nakit bu listeye yetmiyor."));
                            return FString(TEXT("\u00d6deme onayda yap\u0131l\u0131r; koliler yar\u0131n sabah arka kap\u0131da."));
                        }, 10, [G] { return G() && (G()->OrderDraftBill() > G()->State.Cash || (G()->OrderDraftCaseCount() > 0 && G()->OrderDraftBill() < MarketOrderAdvice::MinimumOrder)) ? ERole::Warn : ERole::Muted; })
                    ]
                ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 8.f, 0.f)
                [ Button([] { return FString(TEXT("Temizle")); }, [this] { if (AMarketGameMode* M = Game.Get()) M->ClearOrderDraft(); }, false,
                    [G] { return G() && G()->OrderDraftCaseCount() > 0; }) ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 8.f, 0.f)
                [ Button([] { return FString(TEXT("\u00d6neriyi yaz")); }, [this] { Do(TEXT("SuggestOrder")); }) ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                [ Button([] { return FString(TEXT("Sipari\u015fi onayla")); }, [this] { Do(TEXT("ConfirmOrder")); }, true,
                    [G] { return G() && G()->OrderDraftCaseCount() > 0; }) ],
                ERole::Panel, FMargin(18.f, 12.f))
        ];
}
