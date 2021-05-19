#pragma once

class FGPUDrivenProcess
{
public:
	FGPUDrivenProcess(){
		IsActive = false;
	}
	virtual ~FGPUDrivenProcess(){}

	virtual void Run(IRHIContext* RHIContext, FPSOManager* InPSOManager){}

	void SetActive(bool InActive){IsActive = InActive;}
	bool GetActive(){return IsActive;}

protected:
	std::string PSOName;

private:
	bool IsActive;

};

