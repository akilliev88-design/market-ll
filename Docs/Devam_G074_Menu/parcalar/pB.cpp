// ---------------------------------------------------------------------------------------------------------------
// Theme

bool SMarketMenu::IsLight() const
{
    const AMarketGameMode* G = Game.Get();
    return !G || G->bLightTheme;
}

FLinearColor SMarketMenu::Color(ERole Role) const
{
    using MarketMenuUi::Hex;
    const bool bLight = IsLight();
    switch (Role)
    {
    case ERole::Page: return bLight ? Hex(TEXT("F4F5F7")) : Hex(TEXT("111315"));
    case ERole::Panel: return bLight ? Hex(TEXT("FFFFFF")) : Hex(TEXT("1B1E22"));
    case ERole::Inset: return bLight ? Hex(TEXT("E9ECF0")) : Hex(TEXT("262A30"));
    case ERole::Text: return bLight ? Hex(TEXT("171A1E")) : Hex(TEXT("F4F5F6"));
    case ERole::Muted: return bLight ? Hex(TEXT("5F6670")) : Hex(TEXT("AAB1BA"));
    case ERole::Accent: return bLight ? Hex(TEXT("16775F")) : Hex(TEXT("71C6AC"));
    case ERole::Good: return bLight ? Hex(TEXT("16775F")) : Hex(TEXT("71C6AC"));
    case ERole::Bad: return bLight ? Hex(TEXT("C4453A")) : Hex(TEXT("F07F6E"));
    case ERole::Warn: return bLight ? Hex(TEXT("B7791F")) : Hex(TEXT("F2B45A"));
    case ERole::Line: return bLight ? Hex(TEXT("DDE1E6")) : Hex(TEXT("33383F"));
    case ERole::Button: return bLight ? Hex(TEXT("E9ECEF")) : Hex(TEXT("2C3137"));
    case ERole::ButtonText: return bLight ? Hex(TEXT("171A1E")) : Hex(TEXT("F4F5F6"));
    case ERole::Primary: return bLight ? Hex(TEXT("171A1E")) : Hex(TEXT("71C6AC"));
    case ERole::PrimaryText: return bLight ? Hex(TEXT("FFFFFF")) : Hex(TEXT("0F1A16"));
    case ERole::Dim: return FLinearColor(0.f, 0.f, 0.f, bLight ? 0.35f : 0.55f);
    case ERole::Info: return bLight ? Hex(TEXT("2F5FA8")) : Hex(TEXT("7FA7E0"));
    }
    return FLinearColor::White;
}

TAttribute<FSlateColor> SMarketMenu::Col(ERole Role) const
{
    return TAttribute<FSlateColor>::CreateLambda([this, Role] { return FSlateColor(Color(Role)); });
}

TAttribute<FSlateColor> SMarketMenu::ColBy(TFunction<ERole()> Role) const
{
    return TAttribute<FSlateColor>::CreateLambda([this, Role] { return FSlateColor(Color(Role ? Role() : ERole::Text)); });
}

// ---------------------------------------------------------------------------------------------------------------
// Building blocks

TSharedRef<SWidget> SMarketMenu::Label(TFunction<FString()> Make, int32 Size, ERole Role, bool bBold, bool bWrap)
{
    return SNew(STextBlock).Font(MarketMenuUi::MenuFont(bBold, Size)).ColorAndOpacity(Col(Role)).AutoWrapText(bWrap)
        .Text_Lambda([Make] { return FText::FromString(Make ? Make() : FString()); });
}

TSharedRef<SWidget> SMarketMenu::LabelBy(TFunction<FString()> Make, int32 Size, TFunction<ERole()> Role, bool bBold)
{
    return SNew(STextBlock).Font(MarketMenuUi::MenuFont(bBold, Size)).ColorAndOpacity(ColBy(Role))
        .Text_Lambda([Make] { return FText::FromString(Make ? Make() : FString()); });
}

TSharedRef<SWidget> SMarketMenu::Fixed(const FString& Text, int32 Size, ERole Role, bool bBold)
{
    return SNew(STextBlock).Font(MarketMenuUi::MenuFont(bBold, Size)).ColorAndOpacity(Col(Role)).Text(FText::FromString(Text));
}

TSharedRef<SWidget> SMarketMenu::Card(const TSharedRef<SWidget>& Content, ERole Role, const FMargin& Padding)
{
    return SNew(SBorder).BorderImage(&CardBrush).BorderBackgroundColor(Col(Role)).Padding(Padding)[ Content ];
}

TSharedRef<SWidget> SMarketMenu::Button(TFunction<FString()> Text, TFunction<void()> OnClick, bool bPrimary, TFunction<bool()> Enabled)
{
    return SNew(SButton).ButtonStyle(&PillStyle).IsFocusable(false).ContentPadding(FMargin(16.f, 7.f))
        .HAlign(HAlign_Center).VAlign(VAlign_Center)
        .ButtonColorAndOpacity(Col(bPrimary ? ERole::Primary : ERole::Button))
        .IsEnabled_Lambda([Enabled] { return !Enabled || Enabled(); })
        .OnClicked_Lambda([OnClick] { if (OnClick) OnClick(); return FReply::Handled(); })
    [
        Label(Text, 11, bPrimary ? ERole::PrimaryText : ERole::ButtonText, true)
    ];
}

TSharedRef<SWidget> SMarketMenu::Bar(TFunction<float()> Value, ERole Role)
{
    return SNew(SBox).HeightOverride(8.f)
    [
        SNew(SBorder).BorderImage(&TrackBrush).BorderBackgroundColor(Col(ERole::Line)).Padding(0.f)
        [
        SNew(SProgressBar).Style(&BarStyle).BorderPadding(FVector2D(0.f, 0.f))
        .FillColorAndOpacity(Col(Role))
        .Percent_Lambda([Value]() -> TOptional<float> { return FMath::Clamp(Value ? Value() : 0.f, 0.f, 1.f); })
        ]
    ];
}

const FSlateBrush* SMarketMenu::LogoBrush(const FString& Key)
{
    if (const TSharedPtr<FSlateBrush>* Found = Logos.Find(Key)) return Found->Get();
    TSharedPtr<FSlateBrush> Brush;
    const FString Path = FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Brands"), Key, TEXT("logo.png"));
    if (!Key.IsEmpty() && FPaths::FileExists(Path))
    {
        if (UTexture2D* Texture = FImageUtils::ImportFileAsTexture2D(Path))
        {
            LogoTextures.Emplace(Texture);
            Brush = MakeShared<FSlateBrush>();
            Brush->SetResourceObject(Texture);
            Brush->DrawAs = ESlateBrushDrawType::Image;
            Brush->ImageSize = FVector2D(Texture->GetSizeX(), Texture->GetSizeY());
        }
    }
    Logos.Add(Key, Brush);
    return Brush.Get();
}

TSharedRef<SWidget> SMarketMenu::Badge(const FString& Key, const FString& Initials, const FLinearColor& Fill, float Size)
{
    if (const FSlateBrush* Logo = LogoBrush(Key))
        return SNew(SBox).WidthOverride(Size).HeightOverride(Size)[ SNew(SImage).Image(Logo) ];
    return SNew(SBox).WidthOverride(Size).HeightOverride(Size)
    [
        SNew(SBorder).BorderImage(&BadgeBrush).BorderBackgroundColor(Fill).HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(0.f)
        [
            SNew(STextBlock).Font(MarketMenuUi::MenuFont(true, FMath::RoundToInt32(Size * 0.34f))).ColorAndOpacity(FLinearColor::White)
            .Text(FText::FromString(Initials))
        ]
    ];
}

TSharedRef<SWidget> SMarketMenu::Stat(const FString& Heading, TFunction<FString()> Value, TFunction<FString()> Sub, TFunction<ERole()> ValueRole)
{
    return Card(
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()[ Fixed(Heading, 9, ERole::Muted, true) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)[ LabelBy(Value, 21, ValueRole ? ValueRole : TFunction<ERole()>([] { return ERole::Text; }), true) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)[ Label(Sub, 10, ERole::Muted) ]);
}

TSharedRef<SWidget> SMarketMenu::CategoryChips(FString* Chosen)
{
    TSharedRef<SWrapBox> Box = SNew(SWrapBox).UseAllottedSize(true).InnerSlotPadding(FVector2D(6.f, 6.f));
    TArray<FString> Names = { FString() };
    if (const AMarketGameMode* G = Game.Get()) Names.Append(MarketRivals::Aisles(G->Products));
    for (const FString& Name : Names)
    {
        Box->AddSlot()
        [
            SNew(SButton).ButtonStyle(&PillStyle).IsFocusable(false).ContentPadding(FMargin(13.f, 5.f))
            .ButtonColorAndOpacity_Lambda([this, Chosen, Name] { return FSlateColor(Color(*Chosen == Name ? ERole::Primary : ERole::Button)); })
            .OnClicked_Lambda([Chosen, Name] { *Chosen = Name; return FReply::Handled(); })
            [
                SNew(STextBlock).Font(MarketMenuUi::MenuFont(true, 10))
                .Text(FText::FromString(Name.IsEmpty() ? FString(TEXT("T\u00fcm\u00fc")) : MarketMenuUi::Title(Name)))
                .ColorAndOpacity_Lambda([this, Chosen, Name] { return FSlateColor(Color(*Chosen == Name ? ERole::PrimaryText : ERole::ButtonText)); })
            ]
        ];
    }
    return Box;
}

TSharedRef<SWidget> SMarketMenu::Dot(TFunction<bool()> Done)
{
    return SNew(SBox).WidthOverride(10.f).HeightOverride(10.f).VAlign(VAlign_Center)
    [ SNew(SBorder).BorderImage(&BadgeBrush).BorderBackgroundColor(ColBy([Done] { return Done && Done() ? ERole::Good : ERole::Line; })) ];
}

void SMarketMenu::Do(FName Action, int32 Product)
{
    if (AMarketGameMode* G = Game.Get()) G->MenuCommand(Action, Product);
}

void SMarketMenu::Manage(FName Action, int32 Arg)
{
    if (AMarketGameMode* G = Game.Get()) G->StaffCommand(Action, Arg);
}

void SMarketMenu::Go(int32 Page)
{
    if (AMarketGameMode* G = Game.Get()) G->MenuPage = FMath::Clamp(Page, 0, PageCount - 1);
}

