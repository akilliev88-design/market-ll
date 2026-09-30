TSharedRef<SWidget> SMarketMenu::DayReport()
{
    auto G = [this] { return Game.Get(); };
    TSharedRef<SVerticalBox> Problems = SNew(SVerticalBox);
    for (int32 Index = 0; Index < 3; ++Index)
    {
        auto Problem = [G, Index](MarketDemand::FProblem& Out)
        {
            if (!G()) return false;
            const TArray<MarketDemand::FProblem> All = MarketDemand::TopProblems(G()->State, 3);
            if (!All.IsValidIndex(Index)) return false;
            Out = All[Index];
            return true;
        };
        Problems->AddSlot().AutoHeight().Padding(0.f, 3.f)
        [
            SNew(SBorder).BorderImage(&SmallBrush).BorderBackgroundColor(Col(ERole::Inset)).Padding(FMargin(12.f, 8.f))
            .Visibility_Lambda([Problem] { MarketDemand::FProblem P; return Problem(P) ? EVisibility::Visible : EVisibility::Collapsed; })
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
                [ Label([G, Problem] { MarketDemand::FProblem P; return Problem(P) ? MarketMenuUi::ProblemText(*G(), P) : FString(); }, 11, ERole::Text, false, true) ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(10.f, 0.f, 0.f, 0.f)
                [
                    Button([Problem]
                    {
                        MarketDemand::FProblem P;
                        if (!Problem(P)) return FString();
                        switch (P.Kind)
                        {
                        case MarketDemand::EProblem::Empty: return FString(TEXT("Sipari\u015f ver"));
                        case MarketDemand::EProblem::Expensive: return FString(TEXT("Fiyata bak"));
                        case MarketDemand::EProblem::Waiting: return FString(TEXT("Personel"));
                        default: return FString(TEXT("\u00dcr\u00fcne bak"));
                        }
                    }, [this, Problem]
                    {
                        MarketDemand::FProblem P;
                        if (!Problem(P)) return;
                        if (AMarketGameMode* M = Game.Get(); M && M->Products.IsValidIndex(P.Product)) M->MenuProduct = P.Product;
                        Go(P.Kind == MarketDemand::EProblem::Empty ? Orders : P.Kind == MarketDemand::EProblem::Waiting ? Staff : Prices);
                    })
                ]
            ]
        ];
    }

    return SNew(SScrollBox)
    + SScrollBox::Slot()
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()[ DecisionCard() ]
        + SVerticalBox::Slot().AutoHeight()
        [
            Label([G] { return G() && G()->State.Day > 1 ? FString::Printf(TEXT("%d. g\u00fcn kapand\u0131"), G()->State.Day - 1) : FString(TEXT("Hen\u00fcz kapanm\u0131\u015f g\u00fcn yok. O ile a\u00e7\u0131p g\u00fcn\u00fc bitirince rapor burada.")); }, 13, ERole::Muted, true)
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 12.f, 0.f)
            [ Stat(TEXT("NET SONU\u00c7"), [G] { return G() ? MarketMenuUi::Tl(G()->State.LastProfit) : FString(); },
                   [G] { return G() ? FString::Printf(TEXT("%d k\u00e2rl\u0131 g\u00fcn"), G()->State.ProfitableDays) : FString(); },
                   [G] { return G() && G()->State.LastProfit < 0 ? ERole::Bad : ERole::Good; }) ]
            + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 12.f, 0.f)
            [ Stat(TEXT("C\u0130RO"), [G] { return G() ? MarketMenuUi::Tl(G()->State.LastRevenue) : FString(); },
                   [G] { return G() ? FString::Printf(TEXT("%d sat\u0131\u015f \u00b7 %d kay\u0131p"), G()->State.LastServed, G()->State.LastLost) : FString(); }) ]
            + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 12.f, 0.f)
            [ Stat(TEXT("MAL MAL\u0130YET\u0130"), [G] { return G() ? MarketMenuUi::Tl(G()->State.LastCostOfGoods) : FString(); }, [] { return FString(TEXT("sat\u0131lan \u00fcr\u00fcnlerin al\u0131\u015f\u0131")); }) ]
            + SHorizontalBox::Slot().FillWidth(1.f)
            [ Stat(TEXT("G\u0130DER"), [G] { return G() ? MarketMenuUi::Tl(G()->State.LastOperatingCost) : FString(); },
                   [G] { return G() && G()->State.bSecondStore ? FString::Printf(TEXT("ikinci \u015fube %s"), *MarketMenuUi::Tl(G()->State.LastBranchProfit)) : FString(TEXT("kira, elektrik, maa\u015f")); }) ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.4f).Padding(0.f, 0.f, 12.f, 0.f)
            [
                Card(SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)[ Fixed(TEXT("NEREDE M\u00dc\u015eTER\u0130 KAYBETT\u0130N"), 9, ERole::Muted, true) ]
                    + SVerticalBox::Slot().AutoHeight()
                    [
                        SNew(SBox).Visibility_Lambda([G] { return G() && MarketDemand::TopProblems(G()->State, 1).Num() == 0 ? EVisibility::Visible : EVisibility::Collapsed; })
                        [ Fixed(TEXT("Kay\u0131p m\u00fc\u015fteri yok. Herkes arad\u0131\u011f\u0131n\u0131 buldu."), 11, ERole::Good) ]
                    ]
                    + SVerticalBox::Slot().AutoHeight()[ Problems ])
            ]
            + SHorizontalBox::Slot().FillWidth(1.f)
            [
                Card(SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)[ Fixed(TEXT("YARIN RAK\u0130PLERDE"), 9, ERole::Muted, true) ]
                    + SVerticalBox::Slot().AutoHeight()[ Label([G] { return G() ? G()->RivalNewsText() : FString(); }, 11, ERole::Text, false, true) ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 6.f)[ Fixed(TEXT("MAL KABUL VE BOR\u00c7"), 9, ERole::Muted, true) ]
                    + SVerticalBox::Slot().AutoHeight()
                    [
                        Label([G]
                        {
                            if (!G()) return FString();
                            FString Text = G()->State.DeliveryUnits() > 0 ? FString::Printf(TEXT("%d \u00fcr\u00fcn arka kap\u0131da, depoya ta\u015f\u0131nmal\u0131."), G()->State.DeliveryUnits()) : FString(TEXT("Arka kap\u0131da bekleyen koli yok."));
                            if (G()->State.LastDeliveryMissing + G()->State.LastDeliveryDamaged > 0)
                                Text += FString::Printf(TEXT("\nTedarik sorunu: %d eksik, %d hasarl\u0131."), G()->State.LastDeliveryMissing, G()->State.LastDeliveryDamaged);
                            Text += MarketCampaign::DebtOpen(G()->State) ? FString::Printf(TEXT("\nKalan bor\u00e7 %s."), *MarketMenuUi::Tl(G()->State.InheritedDebt)) : FString(TEXT("\nBor\u00e7 kapand\u0131."));
                            return Text;
                        }, 11, ERole::Text, false, true)
                    ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 6.f)[ Fixed(TEXT("\u0130\u015eLETME: TEDAR\u0130K, PERSONEL, VERG\u0130"), 9, ERole::Muted, true) ]
                    + SVerticalBox::Slot().AutoHeight()
                    [ Label([G] { const FString Text = G() ? MarketDirector::ReportText(G()->State) : FString(); return Text.IsEmpty() ? FString(TEXT("Olay yok.")) : Text; }, 11, ERole::Text, false, true) ])
            ]
        ]
    ];
}

TSharedRef<SWidget> SMarketMenu::WeekReport()
{
    auto G = [this] { return Game.Get(); };
    // Last 7 closed days from the history (oldest left).
    auto DayAt = [G](int32 Slot, FMarketDayRecord& Out)
    {
        if (!G()) return false;
        const TArray<FMarketDayRecord>& History = G()->State.History;
        const int32 Index = History.Num() - 7 + Slot;
        if (!History.IsValidIndex(Index)) return false;
        Out = History[Index];
        return true;
    };
    auto Peak = [G]
    {
        int64 Max = 1;
        if (G())
            for (int32 I = FMath::Max(0, G()->State.History.Num() - 7); I < G()->State.History.Num(); ++I) Max = FMath::Max(Max, FMath::Abs(G()->State.History[I].Profit));
        return Max;
    };
    TSharedRef<SHorizontalBox> Bars = SNew(SHorizontalBox);
    for (int32 Slot = 0; Slot < 7; ++Slot)
    {
        Bars->AddSlot().FillWidth(1.f).Padding(6.f, 0.f)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
            [ Label([DayAt, Slot] { FMarketDayRecord R; return DayAt(Slot, R) ? MarketCatalog::Money(R.Profit) : FString(); }, 9, ERole::Muted) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f)
            [
                SNew(SBox).HeightOverride(150.f).VAlign(VAlign_Bottom)
                [
                    SNew(SBox)
                    .HeightOverride_Lambda([DayAt, Peak, Slot]() -> FOptionalSize { FMarketDayRecord R; return DayAt(Slot, R) ? FMath::Max(3.f, 150.f * static_cast<float>(FMath::Abs(R.Profit)) / static_cast<float>(Peak())) : 0.f; })
                    [
                        SNew(SBorder).BorderImage(&BadgeBrush)
                        .BorderBackgroundColor(ColBy([DayAt, Slot] { FMarketDayRecord R; return DayAt(Slot, R) && R.Profit < 0 ? ERole::Bad : ERole::Accent; }))
                    ]
                ]
            ]
            + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
            [ Label([DayAt, Slot] { FMarketDayRecord R; return DayAt(Slot, R) ? FString::Printf(TEXT("%d. g\u00fcn"), R.Day) : FString(TEXT("\u2014")); }, 10, ERole::Text, true) ]
        ];
    }

    return SNew(SScrollBox)
    + SScrollBox::Slot()
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()
        [
            Label([G] { return G() && G()->State.LastWeekNumber > 0 ? FString::Printf(TEXT("%d. hafta"), G()->State.LastWeekNumber) : FString(TEXT("\u0130lk hafta raporu 7. g\u00fcn\u00fcn sonunda gelir. Grafik \u015fimdiden son g\u00fcnleri g\u00f6sterir.")); }, 13, ERole::Muted, true)
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 12.f, 0.f)
            [ Stat(TEXT("HAFTANIN NET\u0130"), [G] { return G() ? MarketMenuUi::Tl(G()->State.LastWeekProfit) : FString(); }, [G] { return G() ? FString::Printf(TEXT("ciro %s"), *MarketMenuUi::Tl(G()->State.LastWeekRevenue)) : FString(); },
                   [G] { return G() && G()->State.LastWeekProfit < 0 ? ERole::Bad : ERole::Good; }) ]
            + SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 12.f, 0.f)
            [ Stat(TEXT("M\u00dc\u015eTER\u0130"), [G] { return G() ? FString::Printf(TEXT("%d sat\u0131\u015f"), G()->State.LastWeekServed) : FString(); }, [G] { return G() ? FString::Printf(TEXT("%d kay\u0131p m\u00fc\u015fteri"), G()->State.LastWeekLost) : FString(); }) ]
            + SHorizontalBox::Slot().FillWidth(1.f)
            [ Stat(TEXT("\u00d6DENEN BOR\u00c7"), [G] { return G() ? MarketMenuUi::Tl(G()->State.LastWeekDebtPaid) : FString(); },
                   [G] { return !G() ? FString() : MarketCampaign::DebtOpen(G()->State) ? FString::Printf(TEXT("kalan %s"), *MarketMenuUi::Tl(G()->State.InheritedDebt)) : FString::Printf(TEXT("%d. g\u00fcnde kapand\u0131"), G()->State.DebtClearedDay); }) ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
        [
            Card(SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)[ Fixed(TEXT("G\u00dcNL\u00dcK NET \u00b7 SON 7 G\u00dcN"), 9, ERole::Muted, true) ]
                + SVerticalBox::Slot().AutoHeight()[ Bars ]
                + SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
                [
                    Label([DayAt]
                    {
                        FMarketDayRecord First, Last;
                        int32 A = 0;
                        while (A < 7 && !DayAt(A, First)) ++A;
                        if (A >= 7 || !DayAt(6, Last)) return FString();
                        return FString::Printf(TEXT("Yerel pay: ba\u015fta %%%.0f, sonda %%%.0f  \u00b7  kasa: ba\u015fta %s, sonda %s"), First.MarketShare, Last.MarketShare, *MarketMenuUi::Tl(First.Cash), *MarketMenuUi::Tl(Last.Cash));
                    }, 10, ERole::Muted)
                ])
        ]
    ];
}

TSharedRef<SWidget> SMarketMenu::ReportsPage()
{
    auto G = [this] { return Game.Get(); };
    auto Tab = [this](const FString& Text, bool bWeek) -> TSharedRef<SWidget>
    {
        return SNew(SButton).ButtonStyle(&PillStyle).IsFocusable(false).ContentPadding(FMargin(16.f, 6.f))
            .ButtonColorAndOpacity_Lambda([this, bWeek] { return FSlateColor(Color(bWeekTab == bWeek ? ERole::Primary : ERole::Button)); })
            .OnClicked_Lambda([this, bWeek] { bWeekTab = bWeek; return FReply::Handled(); })
            [
                SNew(STextBlock).Font(MarketMenuUi::MenuFont(true, 11)).Text(FText::FromString(Text))
                .ColorAndOpacity_Lambda([this, bWeek] { return FSlateColor(Color(bWeekTab == bWeek ? ERole::PrimaryText : ERole::ButtonText)); })
            ];
    };
    return SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)[ Tab(TEXT("G\u00fcn sonu"), false) ]
            + SHorizontalBox::Slot().AutoWidth()[ Tab(TEXT("Hafta"), true) ]
        ]
        + SVerticalBox::Slot().FillHeight(1.f)
        [
            SNew(SWidgetSwitcher).WidgetIndex_Lambda([this] { return bWeekTab ? 1 : 0; })
            + SWidgetSwitcher::Slot()[ DayReport() ]
            + SWidgetSwitcher::Slot()[ WeekReport() ]
        ]
        + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right).Padding(0.f, 10.f, 0.f, 0.f)
        [
            SNew(SBox).Visibility_Lambda([G] { return G() && G()->bMenuDayReport ? EVisibility::Visible : EVisibility::Collapsed; })
            [ Button([] { return FString(TEXT("Yeni g\u00fcne ba\u015fla")); }, [this] { if (AMarketGameMode* M = Game.Get()) M->CloseMenu(); }, true) ]
        ];
}

void SMarketMenu::ShowWeek(bool bWeek)
{
    bWeekTab = bWeek;
}
