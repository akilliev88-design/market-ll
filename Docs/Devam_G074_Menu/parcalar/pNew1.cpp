
const TCHAR* SMarketMenu::PageName(int32 Page)
{
    switch (Page)
    {
    case Summary: return TEXT("Özet");
    case Orders: return TEXT("Sipariş");
    case Prices: return TEXT("Ürünler ve fiyat");
    case Promotions: return TEXT("Kampanyalar");
    case Rivals: return TEXT("Rakipler");
    case Staff: return TEXT("Personel");
    case Finance: return TEXT("Finans");
    case Channels: return TEXT("Satış kanalları");
    case Branches: return TEXT("Şubeler");
    default: return TEXT("Raporlar");
    }
}

void SMarketMenu::Ask(const FString& Question, TFunction<void()> OnYes)
{
    ConfirmText = Question;
    ConfirmAction = MoveTemp(OnYes);
}

TSharedRef<SWidget> SMarketMenu::RiskyButton(TFunction<FString()> Text, TFunction<FString()> Question, TFunction<void()> OnClick, TFunction<bool()> Enabled)
{
    return Button(Text, [this, Question, OnClick] { Ask(Question ? Question() : FString(TEXT("Emin misin?")), OnClick); }, false, Enabled);
}

TSharedRef<SWidget> SMarketMenu::Choice(const FString& Text, TFunction<bool()> Selected, TFunction<void()> OnClick, TFunction<bool()> Enabled)
{
    return SNew(SButton).ButtonStyle(&PillStyle).IsFocusable(false).ContentPadding(FMargin(12.f, 5.f))
        .ButtonColorAndOpacity_Lambda([this, Selected] { return FSlateColor(Color(Selected && Selected() ? ERole::Primary : ERole::Button)); })
        .IsEnabled_Lambda([Enabled] { return !Enabled || Enabled(); })
        .OnClicked_Lambda([OnClick] { if (OnClick) OnClick(); return FReply::Handled(); })
    [
        SNew(STextBlock).Font(MarketMenuUi::MenuFont(true, 10)).Text(FText::FromString(Text))
        .ColorAndOpacity_Lambda([this, Selected] { return FSlateColor(Color(Selected && Selected() ? ERole::PrimaryText : ERole::ButtonText)); })
    ];
}

TSharedRef<SWidget> SMarketMenu::Section(const FString& Title)
{
    return Fixed(Title, 9, ERole::Muted, true);
}

TSharedRef<SWidget> SMarketMenu::Why(TFunction<FString()> Reason)
{
    return SNew(SBox).Visibility_Lambda([Reason] { return Reason && !Reason().IsEmpty() ? EVisibility::Visible : EVisibility::Collapsed; })
    [ Label(Reason, 9, ERole::Warn, false, true) ];
}

const FSlateBrush* SMarketMenu::PictureBrush(int32 Product)
{
    const AMarketGameMode* G = Game.Get();
    if (!G || !G->Products.IsValidIndex(Product)) return nullptr;
    const FMarketProduct& P = G->Products[Product];
    if (const TSharedPtr<FSlateBrush>* Found = Pictures.Find(P.Id)) return Found->Get();
    TSharedPtr<FSlateBrush> Brush;
    // The studio writes every label to /Game/Products/Items/<id>/T_<id>_Label (MirasMarketStudio).
    const FString Path = FString::Printf(TEXT("/Game/Products/Items/%s/T_%s_Label.T_%s_Label"), *P.Id, *P.Id, *P.Id);
    UTexture2D* Texture = P.MeshPath.IsEmpty() ? nullptr : LoadObject<UTexture2D>(nullptr, *Path, nullptr, LOAD_NoWarn | LOAD_Quiet);
    if (Texture)
    {
        // Box packages: the front face of the atlas (FBoxPackageLayout, the same layout the studio used).
        // Round packages: the middle of the label band, the part the shopper sees.
        FBox2f Region(FVector2f(0.32f, 0.f), FVector2f(0.68f, 1.f));
        FVector2D Size(72.0, 100.0);
        FString Folder;
        const int32 PackagesAt = P.MeshPath.Find(TEXT("/Packages/"));
        if (PackagesAt != INDEX_NONE)
        {
            Folder = P.MeshPath.Mid(PackagesAt + 10);
            int32 Slash = INDEX_NONE;
            if (Folder.FindChar(TEXT('/'), Slash)) Folder = Folder.Left(Slash);
        }
        int32 W = 0, D = 0, H = 0;
        FBoxPackageLayout Layout;
        if (FBoxPackageLayout::ParsePackageId(Folder, W, D, H) && FBoxPackageLayout::Make(W, D, H, Layout))
        {
            const FIntRect Front = Layout.Rects[FBoxPackageLayout::Front];
            const float Atlas = static_cast<float>(FMath::Max(1, Layout.AtlasSize));
            Region = FBox2f(FVector2f(Front.Min.X / Atlas, Front.Min.Y / Atlas), FVector2f(Front.Max.X / Atlas, Front.Max.Y / Atlas));
            Size = FVector2D(FMath::Max(1, Front.Width()), FMath::Max(1, Front.Height()));
        }
        PictureTextures.Emplace(Texture);
        Brush = MakeShared<FSlateBrush>();
        Brush->SetResourceObject(Texture);
        Brush->DrawAs = ESlateBrushDrawType::Image;
        Brush->ImageSize = Size;
        Brush->SetUVRegion(Region);
    }
    Pictures.Add(P.Id, Brush);
    return Brush.Get();
}

TSharedRef<SWidget> SMarketMenu::ProductPicture(int32 Product, float Size)
{
    const AMarketGameMode* G = Game.Get();
    if (const FSlateBrush* Picture = PictureBrush(Product))
    {
        return SNew(SBox).WidthOverride(Size).HeightOverride(Size)
        [
            SNew(SBorder).BorderImage(&SmallBrush).BorderBackgroundColor(Col(ERole::Inset)).Padding(3.f).HAlign(HAlign_Center).VAlign(VAlign_Center)
            [ SNew(SScaleBox).Stretch(EStretch::ScaleToFit)[ SNew(SImage).Image(Picture) ] ]
        ];
    }
    const FString Name = G && G->Products.IsValidIndex(Product) ? G->ProductName(Product) : FString();
    const FLinearColor Fill = G && G->Products.IsValidIndex(Product) ? FLinearColor(G->Products[Product].Color) : Color(ERole::Muted);
    return Badge(FString(), MarketMenuUi::Initials(Name), Fill, Size);
}

// ---------------------------------------------------------------------------------------------------------------
// Frame

void SMarketMenu::Construct(const FArguments& InArgs)
{
    Game = InArgs._Game;
    PillStyle = FButtonStyle().SetNormal(PillBrush).SetHovered(PillHover).SetPressed(PillPress).SetDisabled(PillBrush)
        .SetNormalPadding(FMargin(0.f)).SetPressedPadding(FMargin(0.f));
    RowStyle = FButtonStyle().SetNormal(SmallBrush).SetHovered(SmallHover).SetPressed(SmallPress).SetDisabled(SmallBrush)
        .SetNormalPadding(FMargin(0.f)).SetPressedPadding(FMargin(0.f));
    CircleStyle = FButtonStyle().SetNormal(CircleBrush).SetHovered(CircleBrush).SetPressed(CircleBrush).SetDisabled(CircleBrush)
        .SetNormalPadding(FMargin(0.f)).SetPressedPadding(FMargin(0.f));
    BarStyle = FProgressBarStyle().SetBackgroundImage(NoBrush).SetFillImage(TrackBrush).SetMarqueeImage(TrackBrush);

    ChildSlot
    [
        SNew(SOverlay)
        .Visibility_Lambda([this] { const AMarketGameMode* G = Game.Get(); return G && G->bMenuOpen ? EVisibility::Visible : EVisibility::Collapsed; })
        + SOverlay::Slot()
        [ SNew(SBorder).BorderImage(&FlatBrush).BorderBackgroundColor(Col(ERole::Dim)) ]
        + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(20.f)
        [
            SNew(SBox).WidthOverride(1260.f).HeightOverride(740.f)
            [
                SNew(SOverlay)
                + SOverlay::Slot()
                [
                    SNew(SBorder).BorderImage(&PanelBrush).BorderBackgroundColor(Col(ERole::Page)).Padding(0.f)
                    [
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot().AutoWidth()[ Sidebar() ]
                        + SHorizontalBox::Slot().FillWidth(1.f)
                        [
                            SNew(SVerticalBox)
                            + SVerticalBox::Slot().AutoHeight()[ Header() ]
                            + SVerticalBox::Slot().FillHeight(1.f).Padding(24.f, 0.f, 24.f, 22.f)
                            [
                                SNew(SWidgetSwitcher)
                                .WidgetIndex_Lambda([this] { const AMarketGameMode* G = Game.Get(); return G ? FMath::Clamp(G->MenuPage, 0, PageCount - 1) : 0; })
                                + SWidgetSwitcher::Slot()[ SummaryPage() ]
                                + SWidgetSwitcher::Slot()[ OrdersPage() ]
                                + SWidgetSwitcher::Slot()[ PricesPage() ]
                                + SWidgetSwitcher::Slot()[ PromotionsPage() ]
                                + SWidgetSwitcher::Slot()[ RivalsPage() ]
                                + SWidgetSwitcher::Slot()[ StaffPage() ]
                                + SWidgetSwitcher::Slot()[ FinancePage() ]
                                + SWidgetSwitcher::Slot()[ ChannelsPage() ]
                                + SWidgetSwitcher::Slot()[ BranchesPage() ]
                                + SWidgetSwitcher::Slot()[ ReportsPage() ]
                            ]
                        ]
                    ]
                ]
                + SOverlay::Slot()[ ConfirmLayer() ]
            ]
        ]
    ];
}

TSharedRef<SWidget> SMarketMenu::ConfirmLayer()
{
    return SNew(SOverlay)
        .Visibility_Lambda([this] { return ConfirmAction ? EVisibility::Visible : EVisibility::Collapsed; })
        + SOverlay::Slot()[ SNew(SBorder).BorderImage(&PanelBrush).BorderBackgroundColor(Col(ERole::Dim)) ]
        + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
        [
            SNew(SBox).WidthOverride(460.f)
            [
                Card(SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()[ Fixed(TEXT("EMİN MİSİN?"), 9, ERole::Muted, true) ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 16.f)[ Label([this] { return ConfirmText; }, 13, ERole::Text, false, true) ]
                    + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)
                    [
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
                        [ Button([] { return FString(TEXT("Vazgeç  (Esc)")); }, [this] { ConfirmAction = nullptr; }) ]
                        + SHorizontalBox::Slot().AutoWidth()
                        [ Button([] { return FString(TEXT("Evet  (Enter)")); }, [this] { TFunction<void()> Run = MoveTemp(ConfirmAction); ConfirmAction = nullptr; if (Run) Run(); }, true) ]
                    ],
                    ERole::Panel, FMargin(24.f, 20.f))
            ]
        ];
}

FReply SMarketMenu::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
    AMarketGameMode* G = Game.Get();
    if (!G || !G->bMenuOpen) return FReply::Unhandled();
    const FKey Key = InKeyEvent.GetKey();
    if (ConfirmAction)
    {
        if (Key == EKeys::Escape) { ConfirmAction = nullptr; return FReply::Handled(); }
        if (Key == EKeys::Enter) { TFunction<void()> Run = MoveTemp(ConfirmAction); ConfirmAction = nullptr; if (Run) Run(); return FReply::Handled(); }
        return FReply::Handled();
    }
    if (Key == EKeys::Escape || Key == EKeys::M) { G->CloseMenu(); return FReply::Handled(); }
    static const FKey Digits[PageCount] = { EKeys::One, EKeys::Two, EKeys::Three, EKeys::Four, EKeys::Five, EKeys::Six, EKeys::Seven, EKeys::Eight, EKeys::Nine, EKeys::Zero };
    for (int32 Page = 0; Page < PageCount; ++Page)
        if (Key == Digits[Page]) { Go(Page); return FReply::Handled(); }
    return FReply::Unhandled();
}

FReply SMarketMenu::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    // A click on an empty part must not hand the keyboard to the paused 3D view.
    return FReply::Handled().SetUserFocus(SharedThis(this), EFocusCause::Mouse);
}

TSharedRef<SWidget> SMarketMenu::NavItem(int32 Page, const FString& Text, const FString& Key)
{
    auto IsCurrent = [this, Page] { const AMarketGameMode* G = Game.Get(); return G && G->MenuPage == Page; };
    return SNew(SButton).ButtonStyle(&RowStyle).IsFocusable(false).ContentPadding(FMargin(12.f, 7.f))
        .ButtonColorAndOpacity_Lambda([this, IsCurrent] { return FSlateColor(Color(IsCurrent() ? ERole::Inset : ERole::Panel)); })
        .OnClicked_Lambda([this, Page] { Go(Page); return FReply::Handled(); })
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
        [ SNew(SBox).WidthOverride(12.f)[ Fixed(Key, 9, ERole::Muted, true) ] ]
        + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
        [ LabelBy([Text] { return Text; }, 12, [IsCurrent] { return IsCurrent() ? ERole::Accent : ERole::Text; }, true) ]
    ];
}

TSharedRef<SWidget> SMarketMenu::Sidebar()
{
    auto G = [this] { return Game.Get(); };
    TSharedRef<SVerticalBox> Nav = SNew(SVerticalBox);
    for (int32 Page = 0; Page < PageCount; ++Page)
        Nav->AddSlot().AutoHeight().Padding(0.f, 1.f)[ NavItem(Page, PageName(Page), FString::FromInt((Page + 1) % 10)) ];
    auto Difficulty = [this, G](const FString& Text, int32 Value) -> TSharedRef<SWidget>
    {
        return Choice(Text, [G, Value] { return G() && G()->State.Difficulty == Value; }, [this, Value] { Manage(TEXT("Difficulty"), Value); },
            [G] { return G() && !G()->bOpen; });
    };
    return SNew(SBox).WidthOverride(230.f)
    [
        SNew(SBorder).BorderImage(&PanelBrush).BorderBackgroundColor(Col(ERole::Panel)).Padding(FMargin(14.f, 18.f))
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight().Padding(8.f, 0.f, 0.f, 0.f)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
                [ Badge(TEXT("miras"), TEXT("MM"), Color(ERole::Accent), 34.f) ]
                + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()[ Fixed(TEXT("MİRAS MARKET"), 12, ERole::Text, true) ]
                    + SVerticalBox::Slot().AutoHeight()[ Label([G] { return G() ? FString::Printf(TEXT("Lüleburgaz · %s"), *MarketDirector::DateText(G()->State)) : FString(); }, 9, ERole::Muted) ]
                ]
            ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 16.f, 0.f, 0.f)[ Nav ]
            + SVerticalBox::Slot().FillHeight(1.f)[ SNew(SSpacer) ]
            + SVerticalBox::Slot().AutoHeight().Padding(6.f, 0.f, 0.f, 4.f)[ Section(TEXT("ZORLUK")) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 4.f, 0.f)[ Difficulty(TEXT("Rahat"), 0) ]
                + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 4.f, 0.f)[ Difficulty(TEXT("Normal"), 1) ]
                + SHorizontalBox::Slot().AutoWidth()[ Difficulty(TEXT("Zor"), 2) ]
            ]
            + SVerticalBox::Slot().AutoHeight().Padding(6.f, 0.f, 0.f, 8.f)
            [ Why([G] { return G() && G()->bOpen ? FString(TEXT("Zorluk dükkân kapalıyken değişir.")) : FString(); }) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
            [
                Button([G] { return FString(G() && G()->State.Online.bPandemic ? TEXT("Salgın dönemi: var") : TEXT("Salgın dönemi: yok")); },
                    [this, G] { if (G()) Manage(TEXT("PandemicProfile"), G()->State.Online.bPandemic ? 0 : 1); }, false,
                    [G] { return G() && MarketCalendar::DateOf(G()->State.Day).Year < 2020; })
            ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
            [
                Button([this] { return IsLight() ? FString(TEXT("Koyu tema")) : FString(TEXT("Açık tema")); },
                    [this] { if (AMarketGameMode* Mode = Game.Get()) Mode->ToggleMenuTheme(); })
            ]
            + SVerticalBox::Slot().AutoHeight()
            [
                Button([] { return FString(TEXT("Oyuna dön  (M / Esc)")); }, [this] { if (AMarketGameMode* Mode = Game.Get()) Mode->CloseMenu(); }, true)
            ]
        ]
    ];
}

TSharedRef<SWidget> SMarketMenu::Header()
{
    auto Pill = [this](TFunction<FString()> Make, TFunction<ERole()> Role) -> TSharedRef<SWidget>
    {
        return SNew(SBorder).BorderImage(&PillBrush).BorderBackgroundColor(Col(ERole::Panel)).Padding(FMargin(14.f, 6.f))
            [ LabelBy(Make, 11, Role, true) ];
    };
    return SNew(SHorizontalBox)
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(24.f, 20.f, 16.f, 16.f)
        [
            Label([this] { const AMarketGameMode* G = Game.Get(); return FString(PageName(G ? G->MenuPage : 0)); }, 22, ERole::Text, true)
        ]
        + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center).Padding(0.f, 20.f, 12.f, 16.f)
        [
            SNew(SBox).MaxDesiredWidth(560.f)
            .Visibility_Lambda([this] { const AMarketGameMode* G = Game.Get(); return G && G->MessageTime > 0.f && !G->Message.IsEmpty() ? EVisibility::Visible : EVisibility::Collapsed; })
            [
                SNew(SBorder).BorderImage(&SmallBrush).BorderBackgroundColor(Col(ERole::Inset)).Padding(FMargin(12.f, 6.f))
                [ Label([this] { const AMarketGameMode* G = Game.Get(); return G ? G->Message : FString(); }, 10, ERole::Text, false, true) ]
            ]
        ]
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 20.f, 8.f, 16.f)
        [
            Pill([this] { const AMarketGameMode* G = Game.Get(); return G ? FString::Printf(TEXT("Gün %d  ·  %s"), G->State.Day, G->bOpen ? TEXT("açık, zaman durdu") : TEXT("kapalı")) : FString(); },
                [this] { const AMarketGameMode* G = Game.Get(); return G && G->bOpen ? ERole::Accent : ERole::Muted; })
        ]
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 20.f, 24.f, 16.f)
        [
            Pill([this] { const AMarketGameMode* G = Game.Get(); return G ? TEXT("Kasa  ") + MarketMenuUi::Tl(G->State.Cash) : FString(); },
                [this] { const AMarketGameMode* G = Game.Get(); return G && G->State.Cash < 0 ? ERole::Bad : ERole::Text; })
        ];
}
