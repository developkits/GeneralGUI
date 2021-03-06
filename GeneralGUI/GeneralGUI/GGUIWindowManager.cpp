﻿//-----------------------------------------------------------------------------
// GGUI窗口的容器类
// (C) oil
// 2012-09-16
//-----------------------------------------------------------------------------
#include "GGUIStdAfx.h"
#include "GGUIWindowManager.h"
#include "GGUIDXRenderManager.h"
#include "GGUIWindow.h"
#include "GGUIButton.h"
#include "GGUIPicture.h"
//-----------------------------------------------------------------------------
namespace GGUI
{
	//-----------------------------------------------------------------------------
	GGUIWindowManager::GGUIWindowManager()
	:m_pWindowID2Window(NULL)
	,m_pDelegateID2Delegate(NULL)
	,m_nCapacity(0)
	,m_nIndexEnd(0)
	,m_nDelegateCapacity(0)
	,m_nDelegateIndexEnd(0)
	{

	}
	//-----------------------------------------------------------------------------
	GGUIWindowManager::~GGUIWindowManager()
	{
		ReleaseWindowManager();
	}
	//-----------------------------------------------------------------------------
	bool GGUIWindowManager::InitWindowManager()
	{
		//初始化窗口数组。
		m_nCapacity = 100;
		m_pWindowID2Window = new GGUIWindow*[m_nCapacity];
		memset(m_pWindowID2Window, 0, sizeof(GGUIWindow*)*m_nCapacity);
		//初始化Delegate数组。
		m_nDelegateCapacity = 100;
		m_pDelegateID2Delegate = new stWindowEventDelegate*[m_nDelegateCapacity];
		memset(m_pDelegateID2Delegate, 0, sizeof(stWindowEventDelegate*)*m_nDelegateCapacity);
		return true;
	}
	//-----------------------------------------------------------------------------
	void GGUIWindowManager::ReleaseWindowManager()
	{
		for (SoInt i=0; i<m_nIndexEnd; ++i)
		{
			if (m_pWindowID2Window[i])
			{
				SAFE_DELETE(m_pWindowID2Window[i]);
			}
		}
		SAFE_DELETE_ARRAY(m_pWindowID2Window);
		//
		for (SoInt i=0; i<m_nDelegateIndexEnd; ++i)
		{
			if (m_pDelegateID2Delegate[i])
			{
				SAFE_DELETE(m_pDelegateID2Delegate[i]);
			}
		}
		SAFE_DELETE_ARRAY(m_pDelegateID2Delegate);
	}
	//-----------------------------------------------------------------------------
	void GGUIWindowManager::UpdateWindowManager(SoFloat fFrameTime)
	{
		for (SoInt i=0; i<m_nIndexEnd; ++i)
		{
			if (m_pWindowID2Window[i])
			{
				m_pWindowID2Window[i]->UpdateWindow(fFrameTime);
			}
		}
	}
	//-----------------------------------------------------------------------------
	void GGUIWindowManager::RenderWindowManager()
	{
		//绘制之前，对每个窗口的Mesh顶点做最后的更新。
		PostUpdateWindowManager();
		//
		GGUIDXRenderManager* pDXRenderMgr = GGUIDXRenderManager::GetInstance();
		stRenderUnit theRenderUnit;
		for (SoInt i=0; i<m_nIndexEnd; ++i)
		{
			if (m_pWindowID2Window[i] && m_pWindowID2Window[i]->GetVisible())
			{
				pDXRenderMgr->PreRender();
				m_pWindowID2Window[i]->GenerateRenderUnit(theRenderUnit);
				pDXRenderMgr->AddRnederUnit(theRenderUnit);
				pDXRenderMgr->DoRender();
				pDXRenderMgr->PostRender();
			}
		}
	}
	//-----------------------------------------------------------------------------
	GGUIWindow* GGUIWindowManager::CreateUIWindow(eWindowType theType)
	{
		if (m_nIndexEnd >= m_nCapacity)
		{
			//m_pWindowID2Object容器空间不够了，则把容器空间扩大到原来的2倍。
			SoUInt sizeOfOldArray = sizeof(GGUIWindow*) * m_nCapacity;
			m_nCapacity *= 2;
			GGUIWindow** pNewArray = new GGUIWindow*[m_nCapacity];
			SoUInt sizeOfNewArray = sizeof(GGUIWindow*) * m_nCapacity;
			memset(pNewArray, 0, sizeOfNewArray);
			memcpy_s(pNewArray, sizeOfNewArray, m_pWindowID2Window, sizeOfOldArray);
			SAFE_DELETE_ARRAY(m_pWindowID2Window);
			m_pWindowID2Window = pNewArray;
		}
		GGUIWindow* pNewWindow = NULL;
		switch (theType)
		{
		case WindowType_Base:
			pNewWindow = new GGUIWindow;
			break;
		case WindowType_Button:
			pNewWindow = new GGUIButton;
			break;
		case WindowType_Picture:
			pNewWindow = new GGUIPicture;
			break;
		default:
			break;
		}
		if (pNewWindow)
		{
			m_pWindowID2Window[m_nIndexEnd] = pNewWindow;
			m_pWindowID2Window[m_nIndexEnd]->SetWindowID(m_nIndexEnd);
			++m_nIndexEnd;
		}
		return pNewWindow;
	}
	//-----------------------------------------------------------------------------
	void GGUIWindowManager::ReleaseUIWindow(WindowID theWindowID)
	{
		if (theWindowID >= 0 && theWindowID < m_nIndexEnd)
		{
			if (m_pWindowID2Window[theWindowID])
			{
				SAFE_DELETE(m_pWindowID2Window[theWindowID]);
			}
		}
		else
		{
			//Wait for add log
		}
	}
	//-----------------------------------------------------------------------------
	stWindowEventDelegate* GGUIWindowManager::GetWindowEventDelegate(WindowID theWindowID)
	{
		stWindowEventDelegate* pDelegate = NULL;
		GGUIWindow* pTheWindow = GetUIWindow(theWindowID);
		if (pTheWindow)
		{
			DelegateID theDelegateID = pTheWindow->GetDelegateID();
			if (theDelegateID == Invalid_DelegateID)
			{
				theDelegateID = CreateDelegate();
				pTheWindow->SetDelegateID(theDelegateID);
			}
			if (theDelegateID >= 0 && theDelegateID < m_nDelegateIndexEnd)
			{
				pDelegate = m_pDelegateID2Delegate[theDelegateID];
			}
		}
		return pDelegate;
	}
	//-----------------------------------------------------------------------------
	bool GGUIWindowManager::Next(SoInt& nIndex, GGUIWindow*& pWindow)
	{
		if (nIndex >= 0 && nIndex < m_nIndexEnd)
		{
			if (m_pWindowID2Window[nIndex])
			{
				pWindow = m_pWindowID2Window[nIndex];
				++nIndex;
				return true;
			}
			else
			{
				++nIndex;
				return Next(nIndex, pWindow);
			}
		}
		else
		{
			pWindow = NULL;
			return false;
		}
	}
	//-----------------------------------------------------------------------------
	void GGUIWindowManager::PostUpdateWindowManager()
	{
		for (SoInt i=0; i<m_nIndexEnd; ++i)
		{
			if (m_pWindowID2Window[i])
			{
				m_pWindowID2Window[i]->PostUpdateWindow();
			}
		}
	}
	//-----------------------------------------------------------------------------
	DelegateID GGUIWindowManager::CreateDelegate()
	{
		if (m_nDelegateIndexEnd >= m_nDelegateCapacity)
		{
			//m_pDelegateID2Delegate容器空间不够了，则把容器空间扩大到原来的2倍。
			SoUInt sizeOfOldArray = sizeof(stWindowEventDelegate*) * m_nDelegateCapacity;
			m_nDelegateCapacity *= 2;
			stWindowEventDelegate** pNewArray = new stWindowEventDelegate*[m_nDelegateCapacity];
			SoUInt sizeOfNewArray = sizeof(stWindowEventDelegate*) * m_nDelegateCapacity;
			memset(pNewArray, 0, sizeOfNewArray);
			memcpy_s(pNewArray, sizeOfNewArray, m_pDelegateID2Delegate, sizeOfOldArray);
			SAFE_DELETE_ARRAY(m_pDelegateID2Delegate);
			m_pDelegateID2Delegate = pNewArray;
		}
		//
		m_pDelegateID2Delegate[m_nDelegateIndexEnd] = new stWindowEventDelegate;
		++m_nDelegateIndexEnd;
		return (m_nDelegateIndexEnd-1);
	}
}
//-----------------------------------------------------------------------------
