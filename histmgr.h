#ifndef __HISTORY_MANAGER_H__
#define __HISTORY_MANAGER_H__

#include "tools.h"
#include "resource.h"
#include <vector>

class HistoryManager
{
public:
	HistoryManager(void) : m_iPos(0) {}

	void AddHistoryState(int nMenuID)
	{
		m_aCommands.resize(m_iPos);
		m_aCommands.push_back(nMenuID);
		m_iPos++;
	}

	BOOL CanUndo(void)
	{
		return m_iPos > 0;
	}

	BOOL CanRedo(void)
	{
		return m_iPos + 1 < m_aCommands.size();
	}

	CString GetUndoDescription(void)
	{
		if (CanUndo())
			return MAKEINTRESOURCE(ID_UNDO_OFFSET + m_aCommands[m_iPos - 1]);
		return _T("");
	}

	CString GetRedoDescription(void)
	{
		if (CanRedo())
			return MAKEINTRESOURCE(ID_UNDO_OFFSET + m_aCommands[m_iPos]);
		return _T("");
	}

private:
	unsigned m_iPos;
	std::vector<int> m_aCommands;
};

#endif
