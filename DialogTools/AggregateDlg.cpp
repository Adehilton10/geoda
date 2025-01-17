/**
 * GeoDa TM, Copyright (C) 2011-2015 by Luc Anselin - all rights reserved
 *
 * This file is part of GeoDa.
 * 
 * GeoDa is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GeoDa is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cfloat>
#include <set>
#include <map>
#include <vector>
#include <wx/wx.h>
#include <wx/xrc/xmlres.h>
#include <wx/msgdlg.h>
#include <wx/button.h>
#include <wx/filedlg.h>
#include <wx/textdlg.h>
#include "AggregateDlg.h"
#include "../DataViewer/DataSource.h"
#include "../DataViewer/OGRTable.h"
#include "../DataViewer/OGRColumn.h"
#include "../DataViewer/TableInterface.h"
#include "../FramesManagerObserver.h"
#include "../FramesManager.h"
#include "../DialogTools/ExportDataDlg.h"
#include "../logger.h"
#include "../GeneralWxUtils.h"
#include "../Project.h"

BEGIN_EVENT_TABLE( AggregationDlg, wxDialog )

	EVT_BUTTON( XRCID("ID_INC_ALL_BUTTON"), AggregationDlg::OnIncAllClick )
	EVT_BUTTON( XRCID("ID_INC_ONE_BUTTON"), AggregationDlg::OnIncOneClick )
	EVT_LISTBOX_DCLICK( XRCID("ID_INCLUDE_LIST"), AggregationDlg::OnIncListDClick )
	EVT_BUTTON( XRCID("ID_EXCL_ALL_BUTTON"), AggregationDlg::OnExclAllClick )
	EVT_BUTTON( XRCID("ID_EXCL_ONE_BUTTON"), AggregationDlg::OnExclOneClick )
	EVT_LISTBOX_DCLICK( XRCID("ID_EXCLUDE_LIST"), AggregationDlg::OnExclListDClick )
	EVT_CHOICE( XRCID("ID_CURRENT_KEY_CHOICE"), AggregationDlg::OnKeyChoice )
	EVT_CHOICE( XRCID("ID_IMPORT_KEY_CHOICE"), AggregationDlg::OnKeyChoice )
	EVT_BUTTON( XRCID("wxID_AGGREGATE"), AggregationDlg::OnOKClick )
	EVT_BUTTON( XRCID("wxID_CLOSE"), AggregationDlg::OnCloseClick )
    EVT_CLOSE( AggregationDlg::OnClose )
END_EVENT_TABLE()

AggregationDlg::AggregationDlg(wxWindow* parent, Project* _project_s, const wxPoint& pos)
: project_s(_project_s), export_dlg(NULL)
{
    wxLogMessage("Open AggregationDlg.");
	SetParent(parent);
    
    table_int = project_s->GetTableInt(),
    frames_manager = project_s->GetFramesManager(),
    
	CreateControls();
	Init();
	wxString nm;
	SetTitle(_("Aggregate - ") + table_int->GetTableName());
	SetPosition(pos);
    Centre();
    
	frames_manager->registerObserver(this);
}

AggregationDlg::~AggregationDlg()
{
    frames_manager->removeObserver(this);
    if (export_dlg) {
        export_dlg->Destroy();
        delete export_dlg;
        export_dlg = NULL;
    }
}

void AggregationDlg::update(FramesManager* o)
{
}

void AggregationDlg::CreateControls()
{
	wxXmlResource::Get()->LoadDialog(this, GetParent(), "ID_AGGREGATE_DATA_DLG");
	//m_input_file_name = wxDynamicCast(FindWindow(XRCID("ID_INPUT_FILE_TEXT")), wxTextCtrl);
	//m_key_val_rb = wxDynamicCast(FindWindow(XRCID("ID_KEY_VAL_RB")), wxRadioButton);
	//m_rec_order_rb = wxDynamicCast(FindWindow(XRCID("ID_REC_ORDER_RB")), wxRadioButton);
	m_current_key = wxDynamicCast(FindWindow(XRCID("ID_CURRENT_KEY_CHOICE")), GdaChoice);
	//m_import_key = wxDynamicCast(FindWindow(XRCID("ID_IMPORT_KEY_CHOICE")), wxChoice);
	m_exclude_list = wxDynamicCast(FindWindow(XRCID("ID_EXCLUDE_LIST")), GdaListBox);
	m_include_list = wxDynamicCast(FindWindow(XRCID("ID_INCLUDE_LIST")), GdaListBox);
	m_count = wxDynamicCast(FindWindow(XRCID("ID_AGGREGATE_COUNT")), wxRadioButton);
	m_sum = wxDynamicCast(FindWindow(XRCID("ID_AGGREGATE_SUM")), wxRadioButton);
	m_avg = wxDynamicCast(FindWindow(XRCID("ID_AGGREGATE_AVG")), wxRadioButton);
	m_min = wxDynamicCast(FindWindow(XRCID("ID_AGGREGATE_MIN")), wxRadioButton);
	m_max = wxDynamicCast(FindWindow(XRCID("ID_AGGREGATE_MAX")), wxRadioButton);
	//m_overwrite_field = wxDynamicCast(FindWindow(XRCID("ID_MERGE_OVERWRITE_SAME_FIELD")), wxCheckBox);
    m_inc_all = wxDynamicCast(FindWindow(XRCID("ID_INC_ALL_BUTTON")), wxButton);
    m_inc_one = wxDynamicCast(FindWindow( XRCID("ID_INC_ONE_BUTTON")), wxButton);
    m_exc_all = wxDynamicCast(FindWindow( XRCID("ID_EXCL_ALL_BUTTON")), wxButton);
    m_exc_one = wxDynamicCast(FindWindow( XRCID("ID_EXCL_ONE_BUTTON")), wxButton);
    
    wxScrolledWindow* win = wxDynamicCast(FindWindow( XRCID("ID_AGGREGATE_SCROLL_WIN")), wxScrolledWindow);
   
    win->SetAutoLayout(true);
    win->FitInside();
    win->SetScrollRate(5, 5);
    
    FitInside();
    
    m_count->Bind(wxEVT_RADIOBUTTON, &AggregationDlg::OnMethodSelect, this);
    m_sum->Bind(wxEVT_RADIOBUTTON, &AggregationDlg::OnMethodSelect, this);
    m_avg->Bind(wxEVT_RADIOBUTTON, &AggregationDlg::OnMethodSelect, this);
    m_min->Bind(wxEVT_RADIOBUTTON, &AggregationDlg::OnMethodSelect, this);
    m_max->Bind(wxEVT_RADIOBUTTON, &AggregationDlg::OnMethodSelect, this);
}

void AggregationDlg::Init()
{
    m_current_key->Clear();
    m_include_list->Clear();
    m_exclude_list->Clear();
  
    m_exclude_list->GdaInitContent(table_int,
                                   GdaListBox::SHOW_INTEGER |
                                   GdaListBox::SHOW_FLOAT);
    m_current_key->GdaInitContent(table_int, GdaChoice::SHOW_INTEGER |
                                  GdaChoice::SHOW_STRING);
	UpdateMergeButton();
}


void AggregationDlg::OnMethodSelect( wxCommandEvent& ev)
{
    UpdateMergeButton();
}

void AggregationDlg::OnIncAllClick( wxCommandEvent& ev)
{
    wxLogMessage("Entering AggregationDlg::OnIncAllClick()");
    for (int i=0; i< m_exclude_list->GetCount(); ++i) {
        int col_id = wxNOT_FOUND;
        int tm_id = wxNOT_FOUND;
        wxString sel_str = m_exclude_list->GdaGetString(i, col_id, tm_id);
        if (sel_str.IsEmpty() == false &&
            sel_str != m_current_key->GetString(m_current_key->GetSelection())) {
            m_include_list->GdaAppend(sel_str, col_id, tm_id);
        }
    }
    for (int i=m_exclude_list->GetCount()-1; i>=0; --i) {
        int col_id = wxNOT_FOUND;
        int tm_id = wxNOT_FOUND;
        wxString sel_str = m_exclude_list->GdaGetString(i, col_id, tm_id);
        if (sel_str.IsEmpty() == false &&
            sel_str != m_current_key->GetString(m_current_key->GetSelection())) {
            m_exclude_list->Delete(i);
        }
    }
	UpdateMergeButton();
}

void AggregationDlg::OnIncOneClick( wxCommandEvent& ev)
{
    wxLogMessage("Entering AggregationDlg::OnIncOneClick()");
	if (m_exclude_list->GetSelection() >= 0) {
        int sel_id = m_exclude_list->GetSelection();
        int col_id, tm_id;
        wxString k = m_exclude_list->GdaGetString(sel_id, col_id, tm_id);
        if (k != m_current_key->GetString(m_current_key->GetSelection())) {
            m_include_list->GdaAppend(k, col_id, tm_id);
            m_exclude_list->Delete(sel_id);
        }
	}
	UpdateMergeButton();
}

void AggregationDlg::OnIncListDClick( wxCommandEvent& ev)
{
    wxLogMessage("Entering AggregationDlg::OnIncListDClick()");
	OnExclOneClick(ev);
}

void AggregationDlg::OnExclAllClick( wxCommandEvent& ev)
{
    wxLogMessage("Entering AggregationDlg::OnExclAllClick()");
    for (int i=0, iend=m_include_list->GetCount(); i<iend; i++) {
        int col_id, tm_id;
        wxString k = m_include_list->GdaGetString(i, col_id, tm_id);
        m_exclude_list->GdaAppend(k, col_id, tm_id);
    }
    m_include_list->Clear();
	UpdateMergeButton();
}

void AggregationDlg::OnExclOneClick( wxCommandEvent& ev)
{
    wxLogMessage("Entering AggregationDlg::OnExclOneClick()");
    if (m_include_list->GetSelection() >= 0) {
        int i = m_include_list->GetSelection();
        int col_id, tm_id;
        wxString k = m_include_list->GdaGetString(i, col_id, tm_id);
        m_exclude_list->GdaAppend(k, col_id, tm_id);
        m_include_list->Delete(i);
    }
	UpdateMergeButton();
}

void AggregationDlg::OnExclListDClick( wxCommandEvent& ev)
{
    wxLogMessage("Entering AggregationDlg::OnExclListDClick()");
	OnIncOneClick(ev);
}

bool AggregationDlg::CheckKeys(wxString key_name, std::vector<wxString>& key_vec,
                               std::map<int, std::vector<int> >& key_map)
{
    std::map<wxString, std::vector<int> > dup_dict; // value:[]
    std::vector<wxString> uniq_fnames;
    
    for (int i=0, iend=key_vec.size(); i<iend; i++) {
        wxString tmpK = key_vec[i];
        tmpK.Trim(false);
        tmpK.Trim(true);
        if (dup_dict.find(tmpK) == dup_dict.end()) {
            dup_dict[tmpK].push_back(i);
            uniq_fnames.push_back(tmpK);
        } else {
            dup_dict[tmpK].push_back(i);
        }
    }
    if (key_vec.size() == dup_dict.size()) {
        wxString msg = wxString::Format(_("Your table cannot be aggregated because the key field \"%s\" is unique. Please use another key."), key_name);
        
        wxMessageDialog dlg(this, msg, _("Error"), wxOK | wxICON_ERROR);
        dlg.ShowModal();
        return false;
    }
    for (int i=0; i<uniq_fnames.size(); i++) {
        key_map[i] = dup_dict[uniq_fnames[i]];
    }
    return true;
}


void AggregationDlg::OnOKClick( wxCommandEvent& ev )
{
    wxLogMessage("In AggregationDlg::OnOKClick()");
    try {
        wxString error_msg;
       
        // get selected field names from merging table
        int pos = m_current_key->GetSelection();
        GdaConst::FieldType key_ftype = m_current_key->GdaGetSelectionFieldType(pos);
        std::map<wxString, std::vector<int> > key1_map;
        key1_map = m_current_key->GdaGetUniqueValues(pos);
        int col_id, tm_id;
        wxString key1_name = m_current_key->GdaGetSelection(col_id, tm_id);
        
        if (key1_map.size() == 0 || key_ftype == GdaConst::unknown_type) {
            wxString msg = _("Chosen key field is not valid. Please select another key field");
            wxMessageDialog dlg(this, msg, _("Error"), wxOK | wxICON_ERROR);
            dlg.ShowModal();
            return;
        }
        
        // create in-memory geometries&table
        int new_rows = key1_map.size();
        OGRTable* mem_table = new OGRTable(new_rows);
        
        std::vector<int> col_ids;
        std::vector<int> tm_ids;
        m_include_list->GdaGetAllItems(col_ids, tm_ids);
        
        int in_cols = col_ids.size();
        std::map<wxString, OGRColumn*> new_fields_dict;
        std::vector<wxString> new_fields;
        
        // create key column
        OGRColumn* key_col;
        std::map<wxString, std::vector<int> >::iterator it;
        size_t i;
        
        if (key_ftype == GdaConst::string_type) {
            key_col = new OGRColumnString(key1_name, 50, 0, new_rows);
            for (i=0, it = key1_map.begin(); it != key1_map.end(); ++it, ++i) {
                key_col->SetValueAt(i, it->first);
            }
        }else if (key_ftype==GdaConst::long64_type){
            key_col = new OGRColumnInteger(key1_name, 18, 0, new_rows);
            for (i=0, it = key1_map.begin(); it != key1_map.end(); ++it, ++i) {
                key_col->SetValueAt(i, it->first);
            }
        } else {
            // should not be here
            delete mem_table;
            return;
        }
        new_fields_dict[key1_name] = key_col;
        new_fields.push_back(key1_name);
        
        // create count column
        OGRColumn* cnt_col = new OGRColumnInteger("AGG_COUNT", 18, 0, new_rows);
        for (i=0, it = key1_map.begin(); it != key1_map.end(); ++it, ++i) {
            cnt_col->SetValueAt(i, (wxInt64)(it->second.size()));
        }
        new_fields_dict[cnt_col->GetName()] = cnt_col;
        new_fields.push_back(cnt_col->GetName());
        
        // get columns from table
        for (i=0; i < in_cols; i++ ) {
            int col_id = col_ids[i];
            int tm_id = tm_ids[i];
            OGRColumn* col = CreateNewOGRColumn(new_rows, col_id, tm_id, key1_map);
            if (col) {
                new_fields_dict[col->GetName()] = col;
                new_fields.push_back(col->GetName());
            }
        }
        
        for (i=0; i<new_fields.size(); i++) {
            mem_table->AddOGRColumn(new_fields_dict[new_fields[i]]);
        }
        
        if (export_dlg != NULL) {
            export_dlg->Destroy();
            delete export_dlg;
        }
        export_dlg = new ExportDataDlg(this, mem_table);
        if (export_dlg->ShowModal() == wxID_OK) {
            //wxMessageDialog dlg(this, _("Successful aggregation."), _("Success"), wxOK);
            //dlg.ShowModal();
        }
        delete mem_table;
    } catch (GdaException& ex) {
        if (ex.type() == GdaException::NORMAL)
            return;
        wxMessageDialog dlg(this, ex.what(), _("Error"), wxOK | wxICON_ERROR);
        dlg.ShowModal();
        return;
    }
	ev.Skip();
}

double AggregationDlg::ComputeAgg(std::vector<double>& vals, std::vector<bool>& undefs, std::vector<int>& ids)
{
    if (m_sum->GetValue()) {
        double v_sum = 0;
        for (int i=0; i<ids.size(); i++) {
            int idx = ids[i];
            if (!undefs[idx]) {
                v_sum += vals[idx];
            }
        }
        return v_sum;
        
    } else if (m_avg->GetValue()) {
        double v_sum = 0;
        int n = 0;
        for (int i=0; i<ids.size(); i++) {
            int idx = ids[i];
            if (!undefs[idx]) {
                v_sum += vals[idx];
                n += 1;
            }
        }
        return n==0 ? 0 : v_sum / (double)n;
        
    } else if (m_max->GetValue()) {
        double v_max = DBL_MIN;
        for (int i=0; i<ids.size(); i++) {
            int idx = ids[i];
            if (!undefs[idx]) {
                if (vals[idx] > v_max)
                    v_max = vals[idx];
            }
        }
        return v_max;
    }
    
    else if (m_min->GetValue()) {
        double v_min = DBL_MAX;
        for (int i=0; i<ids.size(); i++) {
            int idx = ids[i];
            if (!undefs[idx]) {
                if (vals[idx] < v_min)
                    v_min = vals[idx];
            }
        }
        return v_min;
    }
	return 0;
}

OGRColumn* AggregationDlg::CreateNewOGRColumn(int new_rows, int col_id, int tm_id,
                                              std::map<wxString, std::vector<int> >& key_map)
{
    int f_length = table_int->GetColLength(col_id, tm_id);
    int f_decimal = table_int->GetColDecimals(col_id, tm_id);
    GdaConst::FieldType f_type = table_int->GetColType(col_id, tm_id);
    wxString f_name = table_int->GetColName(col_id, tm_id);
    std::map<wxString, std::vector<int> >::iterator it;
    size_t cnt = 0;
    std::vector<double> vals;
    std::vector<bool> undefs;
    
    OGRColumn* _col = NULL;
    if (f_type == GdaConst::long64_type) {
        bool is_integer = false;
        if (m_max->GetValue() || m_min->GetValue() || m_sum->GetValue()) {
            _col = new OGRColumnInteger(f_name, f_length, f_decimal, new_rows);
            is_integer = true;
        } else {
            _col = new OGRColumnDouble(f_name, GdaConst::default_dbf_double_len,
                                       GdaConst::default_dbf_double_decimals,
                                       new_rows);
        }
        cnt = 0;
        table_int->GetColData(col_id, tm_id, vals, undefs);
        
        for (it = key_map.begin(); it != key_map.end(); ++it) {
            double v = ComputeAgg(vals, undefs, it->second);
            if (is_integer) {
                wxInt64 vv = v;
                _col->SetValueAt(cnt, vv);
            } else {
                _col->SetValueAt(cnt, v);
            }
            cnt += 1;
        }
        
    } else if (f_type == GdaConst::double_type) {
        _col = new OGRColumnDouble(f_name, f_length, f_decimal, new_rows);
        cnt = 0;
        table_int->GetColData(col_id, tm_id, vals, undefs);
        for (it = key_map.begin(); it != key_map.end(); ++it) {
            std::vector<int>& ids = it->second;
            double v = ComputeAgg(vals, undefs, ids);
            _col->SetValueAt(cnt, v);
            cnt += 1;
        }
        
    }
    return _col;
}

void AggregationDlg::OnCloseClick( wxCommandEvent& ev )
{
    wxLogMessage("In AggregationDlg::OnCloseClick()");
    if (export_dlg) {
        export_dlg->Destroy();
        delete export_dlg;
        export_dlg = NULL;
    }
	EndDialog(wxID_CLOSE);
}

void AggregationDlg::OnClose( wxCloseEvent& ev)
{
    wxLogMessage("In AggregationDlg::OnClose()");
    if (export_dlg) {
        export_dlg->Destroy();
        delete export_dlg;
        export_dlg = NULL;
    }
    Destroy();
}

void AggregationDlg::OnKeyChoice( wxCommandEvent& ev )
{
    wxLogMessage("In AggregationDlg::OnKeyChoice()");
	UpdateMergeButton();
}

void AggregationDlg::UpdateMergeButton()
{
	bool enable = m_count->GetValue() || (!m_include_list->IsEmpty() && m_current_key->GetSelection() != wxNOT_FOUND);
	FindWindow(XRCID("wxID_AGGREGATE"))->Enable(enable);
   
    m_inc_all->Enable(!m_count->GetValue());
    m_inc_one->Enable(!m_count->GetValue());
    m_exc_all->Enable(!m_count->GetValue());
    m_exc_one->Enable(!m_count->GetValue());
}
