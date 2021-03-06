﻿
#include "imcrvtip.h"
#include "TextService.h"
#include "EditSession.h"
#include "InputModeWindow.h"

STDAPI CTextService::OnCompositionTerminated(TfEditCookie ecWrite, ITfComposition *pComposition)
{
	_EndCandidateList();
	showcandlist = FALSE;

	if(pComposition != nullptr)
	{
		ITfRange *pRange = nullptr;
		if(SUCCEEDED(pComposition->GetRange(&pRange)) && (pRange != nullptr))
		{
			pRange->SetText(ecWrite, 0, L"", 0);
			SafeRelease(&pRange);
		}
	}
	SafeRelease(&_pComposition);

	_EndInputModeWindow();

	_ResetStatus();

	return S_OK;
}

BOOL CTextService::_IsComposing()
{
	return _pComposition != nullptr;
}

void CTextService::_SetComposition(ITfComposition *pComposition)
{
	_pComposition = pComposition;
}

BOOL CTextService::_IsRangeCovered(TfEditCookie ec, ITfRange *pRangeTest, ITfRange *pRangeCover)
{
	LONG lResult;

	if(FAILED(pRangeCover->CompareStart(ec, pRangeTest, TF_ANCHOR_START, &lResult)) || lResult > 0)
	{
		return FALSE;
	}

	if(FAILED(pRangeCover->CompareEnd(ec, pRangeTest, TF_ANCHOR_END, &lResult)) || lResult < 0)
	{
		return FALSE;
	}

	return TRUE;
}

class CStartCompositionEditSession : public CEditSessionBase
{
public:
	CStartCompositionEditSession(CTextService *pTextService, ITfContext *pContext) : CEditSessionBase(pTextService, pContext)
	{
	}

	// ITfEditSession
	STDMETHODIMP DoEditSession(TfEditCookie ec)
	{
		HRESULT hr = E_FAIL;

		ITfInsertAtSelection *pInsertAtSelection = nullptr;
		if(SUCCEEDED(_pContext->QueryInterface(IID_PPV_ARGS(&pInsertAtSelection))) && (pInsertAtSelection != nullptr))
		{
			ITfRange *pRange = nullptr;
			if(SUCCEEDED(pInsertAtSelection->InsertTextAtSelection(ec, TF_IAS_QUERYONLY, nullptr, 0, &pRange)) && (pRange != nullptr))
			{
				ITfContextComposition *pContextComposition = nullptr;
				if(SUCCEEDED(_pContext->QueryInterface(IID_PPV_ARGS(&pContextComposition))) && (pContextComposition != nullptr))
				{
					ITfComposition *pComposition = nullptr;
					if(SUCCEEDED(pContextComposition->StartComposition(ec, pRange, _pTextService, &pComposition)) && (pComposition != nullptr))
					{
						_pTextService->_SetComposition(pComposition);

						TF_SELECTION tfSelection;
						tfSelection.range = pRange;
						tfSelection.style.ase = TF_AE_NONE;
						tfSelection.style.fInterimChar = FALSE;
						hr = _pContext->SetSelection(ec, 1, &tfSelection);
					}
					SafeRelease(&pContextComposition);
				}
				SafeRelease(&pRange);
			}
			SafeRelease(&pInsertAtSelection);
		}

		return hr;
	}
};

BOOL CTextService::_StartComposition(ITfContext *pContext)
{
	HRESULT hr = E_FAIL;

	try
	{
		CStartCompositionEditSession *pEditSession = new CStartCompositionEditSession(this, pContext);
		pContext->RequestEditSession(_ClientId, pEditSession, TF_ES_SYNC | TF_ES_READWRITE, &hr);
		SafeRelease(&pEditSession);
	}
	catch(...)
	{
	}

	return SUCCEEDED(hr);
}

class CEndCompositionEditSession : public CEditSessionBase
{
public:
	CEndCompositionEditSession(CTextService *pTextService, ITfContext *pContext) : CEditSessionBase(pTextService, pContext)
	{
	}

	// ITfEditSession
	STDMETHODIMP DoEditSession(TfEditCookie ec)
	{
		_pTextService->_TerminateComposition(ec, _pContext);
		return S_OK;
	}
};

void CTextService::_TerminateComposition(TfEditCookie ec, ITfContext *pContext)
{
	if(pContext == nullptr)	//辞書登録用
	{
		return;
	}

	if(_IsComposing())
	{
		_ClearCompositionDisplayAttributes(ec, pContext);
		_pComposition->EndComposition(ec);
	}
	SafeRelease(&_pComposition);
}

void CTextService::_EndComposition(ITfContext *pContext)
{
	HRESULT hr;

	try
	{
		CEndCompositionEditSession *pEditSession = new CEndCompositionEditSession(this, pContext);
		pContext->RequestEditSession(_ClientId, pEditSession, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hr);
		SafeRelease(&pEditSession);
	}
	catch(...)
	{
	}
}

class CClearCompositionEditSession : public CEditSessionBase
{
public:
	CClearCompositionEditSession(CTextService *pTextService, ITfContext *pContext) : CEditSessionBase(pTextService, pContext)
	{
	}

	// ITfEditSession
	STDMETHODIMP DoEditSession(TfEditCookie ec)
	{
		_pTextService->_CancelComposition(ec, _pContext);
		return S_OK;
	}
};

void CTextService::_CancelComposition(TfEditCookie ec, ITfContext *pContext)
{
	TF_SELECTION tfSelection;
	ULONG cFetched = 0;
	if(FAILED(pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &cFetched)))
	{
		return;
	}

	if(cFetched != 1)
	{
		SafeRelease(&tfSelection.range);
		return;
	}

	if(_IsComposing())
	{
		ITfRange *pRange = nullptr;
		if(SUCCEEDED(_pComposition->GetRange(&pRange)) && (pRange != nullptr))
		{
			if(_IsRangeCovered(ec, tfSelection.range, pRange))
			{
				pRange->SetText(ec, 0, L"", 0);

				tfSelection.range->ShiftEndToRange(ec, pRange, TF_ANCHOR_END);
				tfSelection.range->Collapse(ec, TF_ANCHOR_END);

				pContext->SetSelection(ec, 1, &tfSelection);
			}
			SafeRelease(&pRange);
		}
	}

	SafeRelease(&tfSelection.range);

	_TerminateComposition(ec, pContext);
}

void CTextService::_ClearComposition()
{
	HRESULT hr;

	_EndCandidateList();
	showcandlist = FALSE;

	_EndInputModeWindow();

	if(_IsComposing())
	{
		ITfDocumentMgr *pDocumentMgr = nullptr;
		if(SUCCEEDED(_pThreadMgr->GetFocus(&pDocumentMgr)) && (pDocumentMgr != nullptr))
		{
			ITfContext *pContext = nullptr;
			if(SUCCEEDED(pDocumentMgr->GetTop(&pContext)) && (pContext != nullptr))
			{
				_ResetStatus();

				try
				{
					CClearCompositionEditSession *pEditSession = new CClearCompositionEditSession(this, pContext);
					pContext->RequestEditSession(_ClientId, pEditSession, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hr);
					SafeRelease(&pEditSession);
				}
				catch(...)
				{
				}

				SafeRelease(&pContext);
			}
			SafeRelease(&pDocumentMgr);
		}
	}
}
