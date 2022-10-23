


#include "Orb.h"

#include "Blaster/BlasterPlayerState.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"


AOrb::AOrb()
{
 	
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	OrbMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OrbMesh"));
	OrbMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	OrbMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	OrbMesh->SetSimulatePhysics(false);
	SetRootComponent(OrbMesh);
	

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	AreaSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	AreaSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

}


void AOrb::BeginPlay()
{
	Super::BeginPlay();

	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AOrb::OnSphereOverlap);
	
}

void AOrb::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(!HasAuthority()) return;
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if(BlasterCharacter)
	{
		ABlasterPlayerState* BlasterPlayerState = Cast<ABlasterPlayerState>(BlasterCharacter->GetPlayerState());
		if(BlasterPlayerState)
		{
			if(BlasterPlayerState->GetTeam() != Team)
			{
				//UE_LOG(LogTemp, Warning, TEXT("AOrb::OnSphereOverlap: BlasterPlayerState->GetTeam() != Team"))
				BlasterCharacter->ServerAttachOrb(this);
			}
		}
	}
}



void AOrb::Dropped_Implementation(const FVector& InLocation)
{
	if(RootComponent == nullptr) return;
	//UE_LOG(LogTemp, Warning, TEXT("AOrb::Dropped"))
	FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);
	OrbMesh = OrbMesh == nullptr ? Cast<UStaticMeshComponent>(RootComponent) : OrbMesh;
	if(OrbMesh)
	{
		OrbMesh->DetachFromComponent(DetachmentTransformRules);
	}
	SetOwner(nullptr);
	if(DropSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DropSound, GetActorLocation(), GetActorRotation());
	}
	if(!HasAuthority()) return;
	//TODO: If bots are added, needs to find a nearest location in NavMesh that is accessible to bots
	SetActorLocation(InLocation);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AOrb::PickedUp()
{
	if(HasAuthority()) OnOrbPickedUp.Broadcast(this);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AOrb::SetMaterial()
{
	if(OrbMesh)
	{
		if(RedMaterial && Team == ETeams::ET_RedTeam)
		{
			OrbMesh->SetMaterial(0, RedMaterial);
		}
		else if(BlueMaterial && Team == ETeams::ET_BlueTeam)
		{
			OrbMesh->SetMaterial(0, BlueMaterial);
		}
	}
	
}

void AOrb::Destroyed()
{
	
	
	Super::Destroyed();
}


