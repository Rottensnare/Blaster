// Fill out your copyright notice in the Description page of Project Settings.


#include "ChatBox.h"

#include "Components/EditableTextBox.h"

void UChatBox::OnTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	ChatInput->SetText(FText::FromString(""));
}
