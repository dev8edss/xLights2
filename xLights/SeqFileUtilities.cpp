#include "xLightsMain.h"
#include "SeqSettingsDialog.h"
#include "FileConverter.h"
#include "DataLayer.h"

#include "LMSImportChannelMapDialog.h"
#include "SuperStarImportDialog.h"
#include "SaveChangesDialog.h"

#include <wx/wfstream.h>
#include <wx/zipstrm.h>

void xLightsFrame::NewSequence()
{
    // close any open sequences
    if (!CloseSequence()) {
        return;
    }

    // assign global xml file object
    wxFileName xml_file;
    xml_file.SetPath(CurrentDir);
    CurrentSeqXmlFile = new xLightsXmlFile(xml_file);

    SeqSettingsDialog setting_dlg(this, CurrentSeqXmlFile, mediaDirectory, wxT(""), true);
    setting_dlg.Fit();
    int ret_code = setting_dlg.ShowModal();
    if( ret_code == wxID_CANCEL )
    {
        delete CurrentSeqXmlFile;
        CurrentSeqXmlFile = NULL;
        return;
    }

    // load media if available
    if( CurrentSeqXmlFile->GetSequenceType() == "Media" && CurrentSeqXmlFile->HasAudioMedia() )
    {
        SetMediaFilename(CurrentSeqXmlFile->GetMediaFile());
    }

    wxString mss = CurrentSeqXmlFile->GetSequenceTiming();
    int ms = atoi(mss.c_str());
    LoadSequencer(*CurrentSeqXmlFile);
    CurrentSeqXmlFile->SetSequenceLoaded(true);
    wxString new_timing = "New Timing";
    CurrentSeqXmlFile->AddNewTimingSection(new_timing, this);
    mSequenceElements.AddTimingToAllViews(new_timing);
    MenuItem_File_Save_Sequence->Enable(true);
    MenuItem_File_Close_Sequence->Enable(true);

    if( (NetInfo.GetTotChannels() > SeqData.NumChannels()) ||
        (CurrentSeqXmlFile->GetSequenceDurationMS() / ms) > SeqData.NumFrames() )
    {
        SeqData.init(NetInfo.GetTotChannels(), mMediaLengthMS / ms, ms);
    }
    else
    {
        SeqData.init(NetInfo.GetTotChannels(), CurrentSeqXmlFile->GetSequenceDurationMS() / ms, ms);
    }
    displayElementsPanel->Initialize();
}

static wxFileName mapFileName(const wxFileName &orig) {
    if (orig.GetDirCount() == 0) {
        //likely a filename from windows on Mac/Linux or vice versa
        int idx = orig.GetFullName().Last('\\');
        if (idx == -1) {
            idx = orig.GetFullName().Last('/');
        }
        if (idx != -1) {
            return wxFileName(orig.GetFullName().Left(idx),
                              orig.GetFullName().Right(orig.GetFullName().Length() - idx - 1));
        }
    }
    return orig;
}

void xLightsFrame::OpenSequence()
{
    bool loaded_xml = false;
    bool loaded_fseq = false;
    wxString wildcards = "XML files (*.xml)|*.xml|FSEQ files (*.fseq)|*.fseq";
    wxString filename = wxFileSelector("Choose sequence file to open", CurrentDir, wxEmptyString, "*.xml", wildcards, wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if ( !filename.empty() )
    {
        // close any open sequences
        if (!CloseSequence()) {
            return;
        }

        wxStopWatch sw; // start a stopwatch timer

        wxFileName selected_file(filename);
        wxFileName fseq_file = selected_file;
        fseq_file.SetExt("fseq");
        wxFileName xml_file = selected_file;
        xml_file.SetExt("xml");
        wxFileName media_file;

        // load the fseq data file if it exists
        xlightsFilename = fseq_file.GetFullPath();
        if( fseq_file.FileExists() )
        {
            wxString mf;
            ConvertParameters read_params(xlightsFilename,                              // input filename
                                          SeqData,                                      // sequence data object
                                          GetNetInfo(),                                 // global network info
                                          ConvertParameters::READ_MODE_LOAD_MAIN,       // file read mode
                                          this,                                         // xLights main frame
                                          &mf );                                        // media filename

            FileConverter::ReadFalconFile(read_params);
            if( mf != "" )
            {
                media_file = mapFileName(wxFileName::FileName(mf));
            }
            DisplayXlightsFilename(xlightsFilename);
            SeqBaseChannel=1;
            SeqChanCtrlBasic=false;
            SeqChanCtrlColor=false;
            loaded_fseq = true;
        }

        // assign global xml file object
        CurrentSeqXmlFile = new xLightsXmlFile(xml_file);

        // open the xml file so we can see if it has media
        CurrentSeqXmlFile->Open();

        // if fseq didn't have media check xml
        if( CurrentSeqXmlFile->HasAudioMedia()
           || !CurrentSeqXmlFile->GetMediaFile().IsEmpty())
        {
            media_file = mapFileName(CurrentSeqXmlFile->GetMediaFile());
        }


        // still no media file?  look for an XSEQ file and load if found
        if( !wxFileName(media_file).Exists() )
        {
            wxFileName xseq_file = selected_file;
            xseq_file.SetExt("xseq");
            if( xseq_file.FileExists() )
            {
                wxString mf;
                ReadXlightsFile(xseq_file.GetFullPath(), &mf);
                if( mf != "" )
                {
                    media_file = mapFileName(wxFileName::FileName(mf));
                }
                DisplayXlightsFilename(xlightsFilename);
                SeqBaseChannel=1;
                SeqChanCtrlBasic=false;
                SeqChanCtrlColor=false;
            }
        }

        // double-check file existence
        if( !wxFileName(media_file).Exists() )
        {
            wxFileName detect_media(media_file);

            // search media directory
            detect_media.SetPath(mediaDirectory);
            if( detect_media.FileExists() )
            {
                media_file = detect_media;
            }
            else
            {
                // search selected file directory
                detect_media.SetPath(selected_file.GetPath());
                if( detect_media.FileExists() )
                {
                    media_file = detect_media;
                }
            }
        }

        // search for missing media file in media directory and show directory
        if( !wxFileName(media_file).Exists() )
        {
            wxFileName detect_media(selected_file);
            detect_media.SetExt("mp3");

            // search media directory
            detect_media.SetPath(mediaDirectory);
            if( detect_media.FileExists() )
            {
                media_file = detect_media;
            }
            else
            {
                // search selected file directory
                detect_media.SetPath(selected_file.GetPath());
                if( detect_media.FileExists() )
                {
                    media_file = detect_media;
                }
            }
        }

        // if fseq or xseq had media update xml
        if( !CurrentSeqXmlFile->HasAudioMedia() && wxFileName(media_file).Exists() )
        {
            CurrentSeqXmlFile->SetMediaFile(media_file.GetFullPath(), true);
            int length_ms = Waveform::GetLengthOfMusicFileInMS(media_file.GetFullPath());
            CurrentSeqXmlFile->SetSequenceDurationMS(length_ms);
        }

        if( CurrentSeqXmlFile->WasConverted() )
        {
            SeqSettingsDialog setting_dlg(this, CurrentSeqXmlFile, mediaDirectory, wxT("V3 file was converted. Please check settings!"));
            setting_dlg.Fit();
            setting_dlg.ShowModal();
        }

        wxString mss = CurrentSeqXmlFile->GetSequenceTiming();
        int ms = atoi(mss.c_str());
        loaded_xml = SeqLoadXlightsFile(*CurrentSeqXmlFile, true);

        if( (NetInfo.GetTotChannels() > SeqData.NumChannels()) ||
            (CurrentSeqXmlFile->GetSequenceDurationMS() / ms) > SeqData.NumFrames() )
        {
            SeqData.init(NetInfo.GetTotChannels(), mMediaLengthMS / ms, ms);
        }
        else if( !loaded_fseq )
        {
            SeqData.init(NetInfo.GetTotChannels(), CurrentSeqXmlFile->GetSequenceDurationMS() / ms, ms);
        }
        displayElementsPanel->Initialize();

        if( loaded_fseq )
        {
            bbPlayPause->SetBitmap(playIcon);
            SliderPreviewTime->SetValue(0);
            TextCtrlPreviewTime->Clear();
            UpdateModelsList();
            UpdatePreview();
            Timer1.Start(SeqData.FrameTime());
        }
        else if( !loaded_xml )
        {
            StatusBar1->SetStatusText(wxString::Format("Failed to load: '%s'.", filename));
            return;
        }

        float elapsedTime = sw.Time()/1000.0; //msec => sec
        StatusBar1->SetStatusText(wxString::Format("'%s' loaded in %4.3f sec.", filename, elapsedTime));
        EnableSequenceControls(true);
    }
}

bool xLightsFrame::CloseSequence()
{
    if( mSavedChangeCount !=  mSequenceElements.GetChangeCount() )
    {
        SaveChangesDialog* dlg = new SaveChangesDialog(this);
        if( dlg->ShowModal() == wxID_CANCEL )
        {
            return false;
        }
        if( dlg->GetSaveChanges() )
        {
            SaveSequence();
        }
    }

    // clear everything to prepare for new sequence
    sEffectAssist->SetEffect(NULL);
    xlightsFilename = "";
    mediaFilename.Clear();
    previewLoaded = false;
    previewPlaying = false;
    ResetTimer(NO_SEQ);
    playType = 0;
    selectedEffect = NULL;
    if( CurrentSeqXmlFile )
    {
        delete CurrentSeqXmlFile;
        CurrentSeqXmlFile = NULL;
    }
    mSequenceElements.Clear();
    mSavedChangeCount = mSequenceElements.GetChangeCount();

    mainSequencer->PanelWaveForm->CloseMediaFile();

    EnableSequenceControls(true);  // let it re-evaluate menu state
    MenuSettings->Enable(ID_MENUITEM_RENDER_MODE, false);
    Menu_Settings_Sequence->Enable(false);
    return true;
}

bool xLightsFrame::SeqLoadXlightsFile(const wxString& filename, bool ChooseModels)
{
    delete xLightsFrame::CurrentSeqXmlFile;
    xLightsFrame::CurrentSeqXmlFile = new xLightsXmlFile(filename);
    return SeqLoadXlightsFile(*xLightsFrame::CurrentSeqXmlFile, ChooseModels);
}

// Load the xml file containing effects for a particular sequence
// Returns true if file exists and was read successfully
bool xLightsFrame::SeqLoadXlightsFile(xLightsXmlFile& xml_file, bool ChooseModels )
{
    if( xml_file.IsOpen() )
    {
        LoadSequencer(xml_file);
        xml_file.SetSequenceLoaded(true);
        return true;
    }

    return false;
}

void xLightsFrame::ClearSequenceData()
{
    for( int i = 0; i < SeqData.NumFrames(); ++i)
        for( int j = 0; j < SeqData.NumChannels(); ++j )
            SeqData[i][j] = 0;
}

void xLightsFrame::RenderIseqData(bool bottom_layers)
{
    DataLayerSet& data_layers = CurrentSeqXmlFile->GetDataLayers();
    ConvertParameters::ReadMode read_mode;
    if (bottom_layers && data_layers.GetNumLayers() == 1 &&
        data_layers.GetDataLayer(0)->GetName() == "Nutcracker") {
        DataLayer* nut_layer = data_layers.GetDataLayer(0);
        if( nut_layer->GetDataSource() == xLightsXmlFile::CANVAS_MODE ) {
            //Don't clear, v3 workflow of augmenting the existing fseq file
            return;
        }
    }

    if( bottom_layers )
    {
        ClearSequenceData();
        read_mode = ConvertParameters::READ_MODE_NORMAL;
    }
    else
    {
        read_mode = ConvertParameters::READ_MODE_IGNORE_BLACK;
    }
    int layers_rendered = 0;
    bool start_rendering = bottom_layers;
    for( int i = data_layers.GetNumLayers() - 1; i >= 0; --i )  // build layers bottom up
    {
        DataLayer* data_layer = data_layers.GetDataLayer(i);

        if( data_layer->GetName() != "Nutcracker" )
        {
            if( start_rendering )
            {
                ConvertParameters read_params(data_layer->GetDataSource(),                // input filename
                                              SeqData,                                    // sequence data object
                                              GetNetInfo(),                               // global network info
                                              read_mode,                                  // file read mode
                                              this,                                       // xLights main frame
                                              nullptr,                                    // filename not needed
                                              data_layer );                               // provide data layer for channel offsets

                FileConverter::ReadFalconFile(read_params);
                read_mode = ConvertParameters::READ_MODE_IGNORE_BLACK;
                layers_rendered++;
            }
        }
        else
        {
            if( bottom_layers ) break;  // exit after Nutcracker layer if rendering bottom layers only
            start_rendering = true;
        }
    }
}

void xLightsFrame::SetSequenceEnd(int ms)
{
    mainSequencer->PanelTimeLine->SetSequenceEnd(CurrentSeqXmlFile->GetSequenceDurationMS());
    mSequenceElements.SetSequenceEnd(CurrentSeqXmlFile->GetSequenceDurationMS());
}

static bool CalcPercentage(wxString& value, double base, bool reverse, int offset)
{
    int val = wxAtoi(value);
    val -= offset;
    val %= (int)base;
    if( val < 0 ) return false;
    double half_width = 1.0/base*50.0;
    double percent = (double)val/base*100.0 + half_width;
    if( reverse )
    {
        percent = 100.0 - percent;
    }
    value = wxString::Format("%d",(int)percent);
    return true;
}

static xlColor GetColor(const wxString& sRed, const wxString& sGreen, const wxString& sBlue)
{
    double red,green,blue;
    sRed.ToDouble(&red);
    red = red / 100.0 * 255.0;
    sGreen.ToDouble(&green);
    green = green / 100.0 * 255.0;
    sBlue.ToDouble(&blue);
    blue = blue / 100.0 * 255.0;
    xlColor color(red, green, blue);
    return color;
}
static wxString GetColorString(const wxString& sRed, const wxString& sGreen, const wxString& sBlue)
{
    return (wxString)GetColor(sRed, sGreen, sBlue);
}
static xlColor GetColor(const wxString& rgb) {
    int i = wxAtoi(rgb);
    xlColor cl;
    cl.red = (i & 0xff);
    cl.green = ((i >> 8) & 0xFF);
    cl.blue = ((i >> 16) & 0xff);
    return cl;
}

static EffectLayer* FindOpenLayer(Element* model, int layer_index, int startTimeMS, int endTimeMS, std::vector<bool> &reserved)
{
    EffectLayer* layer;
    int index = layer_index-1;

    layer = model->GetEffectLayer(index);
    if( layer->GetRangeIsClearMS(startTimeMS, endTimeMS) )
    {
        return layer;
    }

    // need to search for open layer
    for( int i = 0; i < model->GetEffectLayerCount(); i++ )
    {
        if (i >= reserved.size() || !reserved[i]) {
            layer = model->GetEffectLayer(i);
            if( layer->GetRangeIsClearMS(startTimeMS, endTimeMS) )
            {
                return layer;
            }
        }
    }

    // empty layer not found so create a new one
    layer = model->AddEffectLayer();
    if (model->GetEffectLayerCount() > reserved.size()) {
        reserved.resize(model->GetEffectLayerCount(), false);
    }
    return layer;
}

#define MAXBUFSIZE 4096
class FixXMLInputStream : public wxInputStream {
public:
    FixXMLInputStream(wxInputStream & in) : wxInputStream(), bin(in) {
    }
    void fillBuf() {
        int pos = bufLen;
        int sz =  MAXBUFSIZE - bufLen;
        bin.Read(&buf[pos], sz);
        size_t ret = bin.LastRead();
        bufLen += ret;

        bool needToClose = false;
        for (int x = 7; x < bufLen; x++) {
            if (buf[x-7] == '<'
                && buf[x-6] == 'p'
                && buf[x-5] == 'i'
                && buf[x-4] == 'x'
                && buf[x-3] == 'e'
                && buf[x-2] == 'l'
                && buf[x-1] == 's'
                && buf[x] == '=') {
                buf[x-2] = ' ';
            } else if (buf[x-7] == '<'
                       && buf[x-6] == 't'
                       && buf[x-5] == 'i'
                       && buf[x-4] == 'm'
                       && buf[x-3] == 'i'
                       && buf[x-2] == 'n'
                       && buf[x-1] == 'g'
                       && buf[x] == ' ') {
                needToClose = true;
            } else if (x > 12 &&
                       buf[x-12] == '<'
                       && buf[x-11] == 'i'
                       && buf[x-10] == 'm'
                       && buf[x-9] == 'a'
                       && buf[x-8] == 'g'
                       && buf[x-7] == 'e'
                       && buf[x-6] == 'A'
                       && buf[x-5] == 'c'
                       && buf[x-4] == 't'
                       && buf[x-3] == 'i'
                       && buf[x-2] == 'o'
                       && buf[x-1] == 'n'
                       && buf[x] == ' ') {
                needToClose = true;
            } else if (x > 11 &&
                       buf[x-11] == '<'
                       && buf[x-10] == 't'
                       && buf[x-9] == 'e'
                       && buf[x-8] == 'x'
                       && buf[x-7] == 't'
                       && buf[x-6] == 'A'
                       && buf[x-5] == 'c'
                       && buf[x-4] == 't'
                       && buf[x-3] == 'i'
                       && buf[x-2] == 'o'
                       && buf[x-1] == 'n'
                       && buf[x] == ' ') {
                needToClose = true;
            } else if (buf[x-1] == '>' && needToClose) {
                if (buf[x-2] != '/') {
                    buf[x - 1] = '/';
                    buf[x] = '>';
                }
                needToClose = false;
            }
        }
    }

    virtual size_t OnSysRead(void *buffer, size_t bufsize) {
        unsigned char *b = (unsigned char *)buffer;
        if (bufsize > 1024) {
            bufsize = 1024;
        }
        size_t ret = 0;
        if (bufLen < 2000) {
            fillBuf();
        }

        if (bufLen) {
            ret = std::min(bufsize, bufLen);
            memcpy(b, buf, ret);
            for (int x = ret; x < bufLen; x++) {
                buf[x-ret] = buf[x];
            }
            bufLen -= ret;
            buf[bufLen] = 0;
            return ret;
        }
        return 0;
    }

private:
    wxBufferedInputStream bin;
    unsigned char buf[MAXBUFSIZE];
    size_t bufLen = 0;
};


void xLightsFrame::OnMenuItemImportEffects(wxCommandEvent& event)
{
    wxFileDialog file(this, "Choose file to import", "", "",
                      _("SuperStar File (*.sup)|*.sup")
                      + "|\nLOR Music Sequences (*.lms)|*.lms"
                      + "|\nxLights Sequence (*.xml)|*.xml"
                      + "|\nHLS hlsIdata Sequences(*.hlsIdata)|*.hlsIdata"
                      + "|\nVixen 2.x Sequence(*.vix)|*.vix"
                      + "|\nLSP 2.x Sequence(*.msq)|*.msq");
    if (file.ShowModal() == wxID_OK) {
        wxFileName fn = file.GetPath();
        if (!fn.Exists()) {
            return;
        }
        if (fn.GetExt() == "lms") {
            ImportLMS(fn);
        } else if (fn.GetExt() == "hlsIdata") {
            ImportHLS(fn);
        } else if (fn.GetExt() == "sup") {
            ImportSuperStar(fn);
        } else if (fn.GetExt() == "vix") {
            ImportVix(fn);
        } else if (fn.GetExt() == "xml") {
            ImportXLights(fn);
        } else if (fn.GetExt() == "msq") {
            ImportLSP(fn);
        }
        mainSequencer->PanelEffectGrid->Refresh();
    }
}

void MapXLightsEffects(EffectLayer *target, EffectLayer *src) {
    for (int x = 0; x < src->GetEffectCount(); x++) {
        Effect *ef = src->GetEffect(x);
        target->AddEffect(0, ef->GetEffectName(), ef->GetSettingsAsString(), ef->GetPaletteAsString(),
                          ef->GetStartTimeMS(), ef->GetEndTimeMS(), 0, 0);
    }
}
void MapXLightsEffects(EffectLayer *target, wxString name, std::map<wxString, EffectLayer *> &layerMap) {
    EffectLayer *src = layerMap[name];
    MapXLightsEffects(target, src);
}
void MapXLightsEffects(Element *target, wxString name, SequenceElements &seqEl, std::map<wxString, EffectLayer *> &layerMap) {
    EffectLayer *src = layerMap[name];
    if (src != nullptr) {
        MapXLightsEffects(target->GetEffectLayer(0), src);
    } else {
        Element * srcEl = seqEl.GetElement(name);
        while (target->GetEffectLayerCount() < srcEl->GetEffectLayerCount()) {
            target->AddEffectLayer();
        }
        for (int x = 0; x < srcEl->GetEffectLayerCount(); x++) {
            MapXLightsEffects(target->GetEffectLayer(x), srcEl->GetEffectLayer(x));
        }
    }
}

void xLightsFrame::ImportXLights(const wxFileName &filename) {
    wxStopWatch sw; // start a stopwatch timer
    std::map<wxString, EffectLayer *> layerMap;

    LMSImportChannelMapDialog dlg(this);
    dlg.mSequenceElements = &mSequenceElements;
    dlg.xlights = this;

    xLightsXmlFile xlf(filename);
    xlf.Open();
    SequenceElements se;
    se.SetFrequency(mSequenceElements.GetFrequency());
    se.SetViewsNode(ViewsNode); // This must come first before LoadSequencerFile.
    se.LoadSequencerFile(xlf);
    for (int e = 0; e < se.GetElementCount(); e++) {
        Element *el = se.GetElement(e);
        bool hasEffects = false;
        for (int l = 0; l < el->GetEffectLayerCount(); l++) {
            hasEffects |= el->GetEffectLayer(l)->GetEffectCount() > 0;
        }
        if (hasEffects) {
            dlg.channelNames.push_back(el->GetName());
        }
        for (int s = 0; s < el->getStrandLayerCount(); s++) {
            StrandLayer *sl = el->GetStrandLayer(s);
            wxString strandName = sl->GetName();
            if (strandName == "") {
                strandName = wxString::Format("Strand %d", (s + 1));
            }
            if (sl->GetEffectCount() > 0) {
                wxString name = sl->GetName();
                dlg.channelNames.push_back(el->GetName() + "/" + strandName);
                layerMap[el->GetName() + "/" + strandName] = sl;
            }
            for (int n = 0; n < sl->GetNodeLayerCount(); n++) {
                NodeLayer *nl = sl->GetNodeLayer(n);
                if (nl->GetEffectCount() > 0) {
                    wxString nodeName = nl->GetName();
                    if (nodeName == "") {
                        nodeName = wxString::Format("Node %d", (n + 1));
                    }
                    dlg.channelNames.push_back(el->GetName() + "/" + strandName + "/" + nodeName);
                    layerMap[el->GetName() + "/" + strandName + "/" + nodeName] = nl;
                }
            }
        }
    }

    dlg.channelNames.Sort();
    dlg.channelNames.Insert("", 0);

    dlg.MapByStrand->Hide();
    dlg.Init();
    // no color colum so remove it and expand the 3rd colum into its space
    dlg.ChannelMapGrid->SetColSize(3, dlg.ChannelMapGrid->GetColSize(3) + dlg.ChannelMapGrid->GetColSize(4));
    dlg.ChannelMapGrid->DeleteCols(4, 1);

    if (dlg.ShowModal() != wxID_OK) {
        return;
    }
    int row = 0;
    for (int m = 0; m < dlg.modelNames.size(); m++) {
        wxString modelName = dlg.modelNames[m];
        ModelClass *mc = GetModelClass(modelName);
        Element * model = nullptr;
        for (int i=0;i<mSequenceElements.GetElementCount();i++) {
            if (mSequenceElements.GetElement(i)->GetType() == "model"
                && modelName == mSequenceElements.GetElement(i)->GetName()) {
                model = mSequenceElements.GetElement(i);
            }
        }
        if (dlg.ChannelMapGrid->GetCellValue(row, 3) != "") {
            MapXLightsEffects(model, dlg.ChannelMapGrid->GetCellValue(row, 3), se, layerMap);
        }
        row++;

        for (int str = 0; str < mc->GetNumStrands(); str++) {
            StrandLayer *sl = model->GetStrandLayer(str);

            if( sl != nullptr ) {
                if ("" != dlg.ChannelMapGrid->GetCellValue(row, 3)) {
                    MapXLightsEffects(sl, dlg.ChannelMapGrid->GetCellValue(row, 3), layerMap);
                }
                row++;
                for (int n = 0; n < mc->GetStrandLength(str); n++) {
                    if ("" != dlg.ChannelMapGrid->GetCellValue(row, 3)) {
                        NodeLayer *nl = sl->GetNodeLayer(n);
                        MapXLightsEffects(nl, dlg.ChannelMapGrid->GetCellValue(row, 3), layerMap);
                    }
                    row++;
                }
            }
        }
    }

    float elapsedTime = sw.Time()/1000.0; //msec => sec
    StatusBar1->SetStatusText(wxString::Format("'%s' imported in %4.3f sec.", filename.GetPath(), elapsedTime));

}

void MapToStrandName(const wxString &name, wxArrayString &strands) {
    if (name.Contains("_")) {
        int idx = name.Find("_") + 1;
        //maybe map to a strand?  Name_0001, Name_0002... etc...

        int ppos = -1;
        int spos = -1;
        for (int x = idx; x < name.size(); x++) {
            if (name[x] == 'P') {
                ppos = x;
            } else if (name[x] == 'S') {
                spos = x;
            } else if (name[x] < '0' || name[x] > '9') {
                return;
            }
        }
        wxString strandName;
        if (spos == -1 && ppos == -1) {
            //simple "strand" names of _#####
            strandName = name.SubString(0, name.Find("_") - 1);
        } else if (spos >= 0 && ppos > spos) {
            //more complex of _S###P###
            strandName = name.SubString(0, ppos - 1);
        }
        if ("" != strandName && strands.Index(strandName) == wxNOT_FOUND) {
            strands.push_back(strandName);
        }
    }
}
void ReadHLSData(wxXmlNode *chand, std::vector<unsigned char> & data) {
    for (wxXmlNode* chani=chand->GetChildren(); chani!=NULL; chani=chani->GetNext()) {
        if ("IlluminationData" == chani->GetName()) {
            for (wxXmlNode* block=chani->GetChildren(); block!=NULL; block=block->GetNext()) {
                wxString vals = block->GetChildren()->GetContent();
                int offset = wxAtoi(vals.SubString(0, vals.Find("-")));
                vals = vals.SubString(vals.Find("-")+1, vals.size());
                while (!vals.IsEmpty()) {
                    wxString v = vals.BeforeFirst(',');
                    vals = vals.AfterFirst(',');
                    long iv = 0;
                    v.ToLong(&iv, 16);
                    data[offset] = iv;
                    offset++;
                }
            }
        }
    }
}
void MapHLSChannelInformation(xLightsFrame *xlights, EffectLayer *layer, wxXmlNode* tuniv, int frames, int frameTime,
                              const wxString &cn, wxColor color, ModelClass &mc, bool byStrand) {
    if (cn == "") {
        return;
    }
    wxXmlNode *redNode = nullptr;
    wxXmlNode *greenNode = nullptr;
    wxXmlNode *blueNode = nullptr;

    for (wxXmlNode* univ=tuniv->GetChildren(); univ!=NULL; univ=univ->GetNext()) {
        if (univ->GetName() == "Universe") {
            for (wxXmlNode* channels=univ->GetChildren(); channels!=NULL; channels=channels->GetNext()) {
                if (channels->GetName() == "Channels") {
                    for (wxXmlNode* chand=channels->GetChildren(); chand!=NULL; chand=chand->GetNext()) {
                        if (chand->GetName() == "ChannelData") {
                            for (wxXmlNode* chani=chand->GetChildren(); chani!=NULL; chani=chani->GetNext()) {
                                if (chani->GetName() == "ChanInfo") {
                                    wxString info = chani->GetChildren()->GetContent();
                                    if (info == cn + ", Normal") {
                                        //single channel, easy
                                        redNode = chand;
                                    } else if (info == cn + ", RGB-R") {
                                        redNode = chand;
                                    } else if (info == cn + ", RGB-G") {
                                        greenNode = chand;
                                    } else if (info == cn + ", RGB-B") {
                                        blueNode = chand;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    if (redNode == nullptr) {
        printf("Did not map %s\n", (const char *)cn.c_str());
        return;
    }
    std::vector<unsigned char> redData(frames);
    std::vector<unsigned char> greenData(frames);
    std::vector<unsigned char> blueData(frames);
    std::vector<xlColor> colors(frames);
    ReadHLSData(redNode, redData);
    if (greenNode != nullptr) {
        ReadHLSData(greenNode, greenData);
        ReadHLSData(blueNode, blueData);
        for (int x = 0; x < frames; x++) {
            colors[x].Set(redData[x], greenData[x], blueData[x]);
        }
    } else {
        xlColor c(color.Red(), color.Green(), color.Blue());
        wxImage::HSVValue hsv = c.asHSV();
        for (int x = 0; x < frames; x++) {
            int i = redData[x];
            //for ramps up/down, HLS does a 1%-100% so the first cell is not linear and
            //we end up not able to map the ramps, we'll try and detect that here
            //and change to 0
            if (i <= 3 && i > 0) {
                if (x < (frames-4)) {
                    if (i < redData[x + 1] && redData[x + 1] < redData[x + 2] && redData[x + 2] < redData[x + 3]) {
                        i = 0;
                    }
                }
                if (x > 4) {
                    if (i < redData[x - 1] && redData[x - 1] < redData[x - 2] && redData[x - 2] < redData[x - 3]) {
                        i = 0;
                    }
                }
            }
            hsv.value = ((double)i) / 255.0;
            colors[x] = hsv;
        }
    }
    xlights->ConvertDataRowToEffects(layer, colors, frameTime);
}
wxString FindHLSStrandName(const wxString &ccrName, int node, const wxArrayString &channelNames) {
    wxString r = ccrName + wxString::Format("P%03d", node);
    if (channelNames.Index(r) == wxNOT_FOUND) {
        r = ccrName + wxString::Format("P%04d", node);
    } else {
        return r;
    }
    if (channelNames.Index(r) == wxNOT_FOUND) {
        r = ccrName + wxString::Format("P%02d", node);
    } else {
        return r;
    }
    if (channelNames.Index(r) == wxNOT_FOUND) {
        r = ccrName + wxString::Format("_%04d", node);
    } else {
        return r;
    }
    if (channelNames.Index(r) == wxNOT_FOUND) {
        r = ccrName + wxString::Format("_%03d", node);
    } else {
        return r;
    }
    if (channelNames.Index(r) == wxNOT_FOUND) {
        return r;
    }
    return "";
}
int base64_decode(const wxString& encoded_string, std::vector<unsigned char> &data);

void MapVixChannelInformation(xLightsFrame *xlights, EffectLayer *layer,
                              std::vector<unsigned char> &data,
                              int frameTime,
                              int numFrames,
                              const wxString & channelName,
                              const wxArrayString &channels,
                              wxColor color,
                              ModelClass &mc) {
    if (channelName == "") {
        return;
    }
    int channel = channels.Index(channelName);
    xlColorVector colors(numFrames);
    if (channel == wxNOT_FOUND) {
        int rchannel = channels.Index(channelName + "Red");
        if (rchannel == wxNOT_FOUND) {
            rchannel = channels.Index(channelName + "-R");
        }
        int gchannel = channels.Index(channelName + "Green");
        if (gchannel == wxNOT_FOUND) {
            gchannel = channels.Index(channelName + "-G");
        }
        int bchannel = channels.Index(channelName + "Blue");
        if (bchannel == wxNOT_FOUND) {
            bchannel = channels.Index(channelName + "-B");
        }
        if (rchannel == wxNOT_FOUND || gchannel == wxNOT_FOUND || bchannel == wxNOT_FOUND) {
            return;
        }
        for (int x = 0; x < numFrames; x++) {
            colors[x].Set(data[x + numFrames * rchannel], data[x + numFrames * gchannel], data[x + numFrames * bchannel]);
        }
    } else {
        xlColor c(color.Red(), color.Green(), color.Blue());
        wxImage::HSVValue hsv = c.asHSV();
        for (int x = 0; x < numFrames; x++) {
            hsv.value = ((double)data[x + numFrames * channel]) / 255.0;
            colors[x] = hsv;
        }
    }
    xlights->ConvertDataRowToEffects(layer, colors, frameTime);
}

// xml
#include "../include/spxml-0.5/spxmlparser.hpp"
#include "../include/spxml-0.5/spxmlevent.hpp"
#ifndef MAX_READ_BLOCK_SIZE
#define MAX_READ_BLOCK_SIZE 4096 * 1024
#endif

void xLightsFrame::ImportVix(const wxFileName &filename) {
    wxStopWatch sw; // start a stopwatch timer

    wxString NodeName,NodeValue,msg;
    std::vector<unsigned char> VixSeqData;
    long cnt = 0;
    wxArrayString context;
    long MaxIntensity = 255;

    int time = 0;
    int frameTime = 0;


    LMSImportChannelMapDialog dlg(this);
    dlg.mSequenceElements = &mSequenceElements;
    dlg.xlights = this;


    SP_XmlPullParser *parser = new SP_XmlPullParser();
    parser->setMaxTextSize(MAX_READ_BLOCK_SIZE / 2);
    wxFile file(filename.GetFullPath());
    char *bytes = new char[MAX_READ_BLOCK_SIZE];
    size_t read = file.Read(bytes, MAX_READ_BLOCK_SIZE);
    parser->append(bytes, read);
    wxString carryOver;

    wxArrayString unsortedChannels;

    int chanColor = -1;

    //pass 1, read the length, determine number of networks, units/network, channels per unit
    SP_XmlPullEvent * event = parser->getNext();
    int done = 0;
    while (!done) {
        if (!event) {
            read = file.Read(bytes, MAX_READ_BLOCK_SIZE);
            if (read == 0) {
                done = true;
            } else {
                parser->append(bytes, read);
            }
        } else {
            switch(event -> getEventType()) {
                case SP_XmlPullEvent::eEndDocument:
                    done = true;
                    break;
                case SP_XmlPullEvent::eStartTag:
                {
                    SP_XmlStartTagEvent * stagEvent = (SP_XmlStartTagEvent*)event;
                    NodeName = stagEvent->getName();
                    context.push_back(NodeName);
                    cnt++;
                    if (cnt > 1 && context[1] == wxString("Channels") && NodeName == wxString("Channel")) {
                        chanColor = wxAtoi(stagEvent -> getAttrValue("color")) & 0xFFFFFF;
                    }
                }
                break;
                case SP_XmlPullEvent::eCData:
                {
                    SP_XmlCDataEvent * stagEvent = (SP_XmlCDataEvent*)event;
                    if (cnt >= 2) {
                        NodeValue = stagEvent->getText();
                        if (context[1] == wxString("MaximumLevel")) {
                            MaxIntensity = wxAtoi(NodeValue);
                        } else if (context[1] == wxString("EventPeriodInMilliseconds")) {
                            frameTime = wxAtoi(NodeValue);
                        } else if (context[1] == wxString("Time")) {
                            time = wxAtoi(NodeValue);
                        } else if (context[1] == wxString("EventValues")) {
                            //AppendConvertStatus(string_format(wxString("Chunk Size=%d\n"), NodeValue.size()));
                            if (carryOver.size() > 0) {
                                NodeValue.insert(0, carryOver);
                            }
                            int i = base64_decode(NodeValue, VixSeqData);
                            if (i != 0) {
                                int start = NodeValue.size() - i - 1;
                                carryOver = NodeValue.substr(start, start + i);
                            } else {
                                carryOver.clear();
                            }
                        } else if (context[1] == wxString("Channels") && context[2] == wxString("Channel")) {
                            dlg.channelNames.push_back(NodeValue);
                            unsortedChannels.push_back(NodeValue);

                            xlColor c(chanColor, false);
                            bool addRGB = false;
                            wxString base;
                            if (NodeValue.EndsWith("Red") || NodeValue.EndsWith("-R")) {
                                c = xlRED;
                                if (NodeValue.EndsWith("-R")) {
                                    base = NodeValue.SubString(0, NodeValue.size() - 3);
                                } else {
                                    base = NodeValue.SubString(0, NodeValue.size() - 4);
                                }
                                if ((dlg.channelNames.Index(base + "Blue") != wxNOT_FOUND
                                     && dlg.channelNames.Index(base + "Green") != wxNOT_FOUND)
                                    || (dlg.channelNames.Index(base + "-B") != wxNOT_FOUND
                                        && dlg.channelNames.Index(base + "-G") != wxNOT_FOUND))
                                {
                                    addRGB = true;
                                }
                            } else if (NodeValue.EndsWith("Blue") || NodeValue.EndsWith("-B")) {
                                c = xlBLUE;
                                if (NodeValue.EndsWith("-B")) {
                                    base = NodeValue.SubString(0, NodeValue.size() - 3);
                                } else {
                                    base = NodeValue.SubString(0, NodeValue.size() - 5);
                                }
                                if ((dlg.channelNames.Index(base + "Red") != wxNOT_FOUND
                                     && dlg.channelNames.Index(base + "Green") != wxNOT_FOUND)
                                    || (dlg.channelNames.Index(base + "-R") != wxNOT_FOUND
                                        && dlg.channelNames.Index(base + "-G") != wxNOT_FOUND)) {
                                    addRGB = true;
                                }
                            } else if (NodeValue.EndsWith("Green") || NodeValue.EndsWith("-G")) {
                                c = xlGREEN;
                                if (NodeValue.EndsWith("-G")) {
                                    base = NodeValue.SubString(0, NodeValue.size() - 3);
                                } else {
                                    base = NodeValue.SubString(0, NodeValue.size() - 6);
                                }
                                if ((dlg.channelNames.Index(base + "Blue") != wxNOT_FOUND
                                     && dlg.channelNames.Index(base + "Red") != wxNOT_FOUND)
                                    || (dlg.channelNames.Index(base + "-B") != wxNOT_FOUND
                                        && dlg.channelNames.Index(base + "-R") != wxNOT_FOUND)) {
                                    addRGB = true;
                                }
                            }
                            dlg.channelColors[NodeValue] = c;
                            if (addRGB) {
                                dlg.channelColors[base] = xlBLACK;
                                dlg.channelNames.push_back(base);
                            }
                        }
                    }
                    break;
                }
                case SP_XmlPullEvent::eEndTag:
                    if (cnt > 0) {
                        context.RemoveAt(cnt-1);
                    }
                    cnt = context.size();
                    break;
            }
            delete event;
        }
        if (!done) {
            event = parser->getNext();
        }
    }
    delete [] bytes;
    delete parser;
    file.Close();

    int numFrames = time / frameTime;

    dlg.ccrNames.Sort();
    dlg.ccrNames.Insert("", 0);

    dlg.channelNames.Sort();
    dlg.channelNames.Insert("", 0);

    dlg.MapByStrand->Hide();
    dlg.Init();

    if (dlg.ShowModal() != wxID_OK) {
        return;
    }

    int row = 0;
    for (int m = 0; m < dlg.modelNames.size(); m++) {
        wxString modelName = dlg.modelNames[m];
        ModelClass *mc = GetModelClass(modelName);
        Element * model = nullptr;
        for (int i=0;i<mSequenceElements.GetElementCount();i++) {
            if (mSequenceElements.GetElement(i)->GetType() == "model"
                && modelName == mSequenceElements.GetElement(i)->GetName()) {
                model = mSequenceElements.GetElement(i);
            }
        }
        MapVixChannelInformation(this, model->GetEffectLayer(0),
                                 VixSeqData, frameTime, numFrames,
                                 dlg.ChannelMapGrid->GetCellValue(row, 3),
                                 unsortedChannels,
                                 dlg.ChannelMapGrid->GetCellBackgroundColour(row, 4),
                                 *mc);
        row++;

        for (int str = 0; str < mc->GetNumStrands(); str++) {
            StrandLayer *sl = model->GetStrandLayer(str);

            if ("" != dlg.ChannelMapGrid->GetCellValue(row, 3)) {
                    MapVixChannelInformation(this, sl,
                                             VixSeqData, frameTime, numFrames,
                                             dlg.ChannelMapGrid->GetCellValue(row, 3),
                                             unsortedChannels,
                                             dlg.ChannelMapGrid->GetCellBackgroundColour(row, 4), *mc);
            }
            row++;
            for (int n = 0; n < mc->GetStrandLength(str); n++) {
                if ("" != dlg.ChannelMapGrid->GetCellValue(row, 3)) {
                    MapVixChannelInformation(this, sl->GetNodeLayer(n),
                                             VixSeqData, frameTime, numFrames,
                                             dlg.ChannelMapGrid->GetCellValue(row, 3),
                                             unsortedChannels,
                                             dlg.ChannelMapGrid->GetCellBackgroundColour(row, 4), *mc);
                }
                row++;
            }
        }
    }


    float elapsedTime = sw.Time()/1000.0; //msec => sec
    StatusBar1->SetStatusText(wxString::Format("'%s' imported in %4.3f sec.", filename.GetPath(), elapsedTime));
}

void xLightsFrame::ImportHLS(const wxFileName &filename)
{
    wxStopWatch sw; // start a stopwatch timer

    wxFileName xml_file(filename);
    wxXmlDocument input_xml;
    wxString xml_doc = xml_file.GetFullPath();
    wxFileInputStream fin(xml_doc);

    if( !input_xml.Load(fin) )  return;

    LMSImportChannelMapDialog dlg(this);
    dlg.mSequenceElements = &mSequenceElements;
    dlg.xlights = this;

    /*
     </ChannelData> | </IlluminationData>
     </Channels>
     </Universe>
     </TotalUniverses> | NumberOfTimeCells | MilliSecPerTimeUnit
     </HLS_OutputSequence>
     */
    int frames = 0;
    int frameTime = 0;
    wxXmlNode *totalUniverses = nullptr;
    for (wxXmlNode* tuniv=input_xml.GetRoot()->GetChildren(); tuniv!=NULL; tuniv=tuniv->GetNext()) {
        if (tuniv->GetName() == "NumberOfTimeCells") {
            frames = wxAtoi(tuniv->GetChildren()->GetContent());
        } else if (tuniv->GetName() == "MilliSecPerTimeUnit") {
            frameTime = wxAtoi(tuniv->GetChildren()->GetContent());
        } else if (tuniv->GetName() == "TotalUniverses") {
            totalUniverses = tuniv;
            for (wxXmlNode* univ=tuniv->GetChildren(); univ!=NULL; univ=univ->GetNext()) {
                if (univ->GetName() == "Universe") {
                    for (wxXmlNode* channels=univ->GetChildren(); channels!=NULL; channels=channels->GetNext()) {
                        if (channels->GetName() == "Channels") {
                            for (wxXmlNode* chand=channels->GetChildren(); chand!=NULL; chand=chand->GetNext()) {
                                if (chand->GetName() == "ChannelData") {
                                    for (wxXmlNode* chani=chand->GetChildren(); chani!=NULL; chani=chani->GetNext()) {
                                        if (chani->GetName() == "ChanInfo") {
                                            wxString info = chani->GetChildren()->GetContent();
                                            if (info.Contains(", Normal")) {
                                                wxString name = info.SubString(0, info.Find(", Normal") - 1);
                                                dlg.channelNames.push_back(name);
                                                dlg.channelColors[name] = xlWHITE;
                                                MapToStrandName(name, dlg.ccrNames);
                                            } else if (info.Contains(", RGB-")) {
                                                wxString name = info.SubString(0, info.Find(", RGB-") - 1);
                                                wxString color = info.GetChar(info.size() - 1);
                                                if (color == "R") {
                                                    dlg.channelNames.push_back(name);
                                                    dlg.channelColors[name] = xlBLACK;
                                                }
                                                dlg.channelNames.push_back(info);
                                                if (color == "R") {
                                                    dlg.channelColors[info] = xlRED;
                                                } else if (color == "G") {
                                                    dlg.channelColors[info] = xlGREEN;
                                                } else {
                                                    dlg.channelColors[info] = xlBLUE;
                                                }
                                                MapToStrandName(name, dlg.ccrNames);
                                            } // else "Dead Channel"
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    dlg.ccrNames.Sort();
    dlg.ccrNames.Insert("", 0);

    dlg.channelNames.Sort();
    dlg.channelNames.Insert("", 0);

    dlg.Init();

    if (dlg.ShowModal() != wxID_OK) {
        return;
    }

    int row = 0;
    for (int m = 0; m < dlg.modelNames.size(); m++) {
        wxString modelName = dlg.modelNames[m];
        ModelClass *mc = GetModelClass(modelName);
        Element * model = nullptr;
        for (int i=0;i<mSequenceElements.GetElementCount();i++) {
            if (mSequenceElements.GetElement(i)->GetType() == "model"
                && modelName == mSequenceElements.GetElement(i)->GetName()) {
                model = mSequenceElements.GetElement(i);
            }
        }
        MapHLSChannelInformation(this, model->GetEffectLayer(0),
                                 totalUniverses, frames, frameTime,
                                 dlg.ChannelMapGrid->GetCellValue(row, 3),
                                 dlg.ChannelMapGrid->GetCellBackgroundColour(row, 4),
                                 *mc, dlg.MapByStrand->GetValue());
        row++;

        for (int str = 0; str < mc->GetNumStrands(); str++) {
            StrandLayer *sl = model->GetStrandLayer(str);

            if ("" != dlg.ChannelMapGrid->GetCellValue(row, 3)) {
                if (!dlg.MapByStrand->GetValue()) {
                    MapHLSChannelInformation(this, sl,
                                             totalUniverses, frames, frameTime,
                                             dlg.ChannelMapGrid->GetCellValue(row, 3),
                                             dlg.ChannelMapGrid->GetCellBackgroundColour(row, 4),
                                             *mc, false);
                } else {
                    wxString ccrName = dlg.ChannelMapGrid->GetCellValue(row, 3);
                    for (int n = 0; n < sl->GetNodeLayerCount(); n++) {
                        EffectLayer *layer = sl->GetNodeLayer(n);

                        wxString nm = FindHLSStrandName(ccrName, n+1, dlg.channelNames);

                        MapHLSChannelInformation(this, layer,
                                                 totalUniverses, frames, frameTime,
                                                 nm,
                                                 dlg.ChannelMapGrid->GetCellBackgroundColour(row, 4),
                                                 *mc, true);



                    }
                }
            }
            row++;
            if (!dlg.MapByStrand->GetValue()) {
                for (int n = 0; n < mc->GetStrandLength(str); n++) {
                    if ("" != dlg.ChannelMapGrid->GetCellValue(row, 3)) {
                        MapHLSChannelInformation(this, sl->GetNodeLayer(n),
                                                 totalUniverses, frames, frameTime,
                                                 dlg.ChannelMapGrid->GetCellValue(row, 3),
                                                 dlg.ChannelMapGrid->GetCellBackgroundColour(row, 4),
                                                 *mc, false);
                    }
                    row++;
                }
            }
        }
    }


    float elapsedTime = sw.Time()/1000.0; //msec => sec
    StatusBar1->SetStatusText(wxString::Format("'%s' imported in %4.3f sec.", filename.GetPath(), elapsedTime));
}

void xLightsFrame::ImportLMS(const wxFileName &filename) {
    wxStopWatch sw; // start a stopwatch timer

    wxFileName xml_file(filename);
    wxXmlDocument input_xml;
    wxString xml_doc = xml_file.GetFullPath();
    wxFileInputStream fin(xml_doc);

    if( !input_xml.Load(fin) )  return;
    ImportLMS(input_xml);
    float elapsedTime = sw.Time()/1000.0; //msec => sec
    StatusBar1->SetStatusText(wxString::Format("'%s' imported in %4.3f sec.", filename.GetPath(), elapsedTime));
}
void xLightsFrame::ImportSuperStar(const wxFileName &filename)
{
    SuperStarImportDialog dlg(this);

    for(int i=0;i<mSequenceElements.GetElementCount();i++) {
        if(mSequenceElements.GetElement(i)->GetType()== "model") {
            dlg.ChoiceSuperStarImportModel->Append(mSequenceElements.GetElement(i)->GetName());
        }
    }

    if (dlg.ShowModal() == wxID_CANCEL) {
        return;
    }
    wxString model_name = dlg.ChoiceSuperStarImportModel->GetStringSelection();
    if( model_name == "" )
    {
        wxMessageBox("Please select the target model!");
        return;
    }

    wxStopWatch sw; // start a stopwatch timer
    bool model_found = false;

    // read v3 xml file into temporary document
    wxFileName xml_file(filename);
    wxXmlDocument input_xml;
    wxString xml_doc = xml_file.GetFullPath();
    wxFileInputStream fin(xml_doc);
    FixXMLInputStream bufIn(fin);

    if( !input_xml.Load(bufIn) )  return;

    Element* model = nullptr;

    for(int i=0;i<mSequenceElements.GetElementCount();i++) {
        if(mSequenceElements.GetElement(i)->GetType()== "model") {
            model = mSequenceElements.GetElement(i);
            if( model->GetName() == model_name ) {
                model_found = true;
                break;
            }
        }
    }
    if( model != nullptr && model_found ) {
        int x_size = wxAtoi(dlg.TextCtrl_SS_X_Size->GetValue());
        int y_size = wxAtoi(dlg.TextCtrl_SS_Y_Size->GetValue());
        int x_offset = wxAtoi(dlg.TextCtrl_SS_X_Offset->GetValue());
        int y_offset = wxAtoi(dlg.TextCtrl_SS_Y_Offset->GetValue());
        bool flip_y = dlg.CheckBox_SS_FlipY->GetValue();
        ImportSuperStar(model, input_xml, x_size, y_size, x_offset, y_offset, flip_y);
    }
    float elapsedTime = sw.Time()/1000.0; //msec => sec
    StatusBar1->SetStatusText(wxString::Format("'%s' imported in %4.3f sec.", filename.GetPath(), elapsedTime));
}

bool findRGB(wxXmlNode *e, wxXmlNode *chan, wxXmlNode *&rchannel, wxXmlNode *&gchannel, wxXmlNode *&bchannel) {
    wxString idxs[3];
    int cnt = 0;
    for (wxXmlNode *n = chan->GetChildren(); n != nullptr; n = n->GetNext()) {
        if (n->GetName() == "channels") {
            for (wxXmlNode *n2 = n->GetChildren(); n2 != nullptr; n2 = n2->GetNext()) {
                if (n2->GetName() == "channel" && cnt < 3) {
                    idxs[cnt] = n2->GetAttribute("savedIndex");
                    cnt++;
                }
            }
        }
    }
    for (wxXmlNode* ch=e->GetChildren(); ch!=NULL; ch=ch->GetNext()) {
        if (ch->GetName() == "channel") {
            wxString idx = ch->GetAttribute("savedIndex");
            if (idx == idxs[0]) {
                rchannel = ch;
            }
            if (idx == idxs[1]) {
                gchannel = ch;
            }
            if (idx == idxs[2]) {
                bchannel = ch;
            }
        }
    }
    return true;
}

void GetRGBTimes(wxXmlNode *re, int &startms, int &endms) {
    if (re != nullptr) {
        startms = wxAtoi(re->GetAttribute("startCentisecond")) * 10;
        endms = wxAtoi(re->GetAttribute("endCentisecond")) * 10;
    } else {
        startms = 9999999;
        endms = 9999999;
    }
}
void GetIntensities(wxXmlNode *re, int &starti, int &endi) {
    wxString intensity = re->GetAttribute("intensity", "-1");
    if (intensity == "-1") {
        starti = wxAtoi(re->GetAttribute("startIntensity"));
        endi = wxAtoi(re->GetAttribute("endIntensity"));
    } else {
        starti = endi = wxAtoi(intensity);
    }
}

class RGBData {
public:
    int startms, endms;
    int starti, endi;
    bool shimmer;
};

void FillData(wxXmlNode *nd, RGBData &data) {
    GetIntensities(nd, data.starti, data.endi);
    GetRGBTimes(nd, data.startms, data.endms);
    data.shimmer = nd->GetAttribute("type") == "shimmer";
}
void Insert(int x, std::vector<RGBData> &v, int startms) {
    v.insert(v.begin() + x, 1, RGBData());
    v[x].startms = startms;
    v[x].endms = v[x + 1].startms;
    v[x].endi = v[x].starti = 0;
    v[x].shimmer = false;
}
void Split(int x, std::vector<RGBData> &v, int endms) {
    v.insert(v.begin() + x, 1, RGBData());
    v[x].startms = v[x + 1].startms;
    v[x].endms = endms;
    v[x + 1].startms = endms;
    v[x].shimmer = v[x + 1].shimmer;
    v[x].starti = v[x + 1].starti;
    double d = endms - v[x].startms;
    d = d / double(v[x + 1].endms - v[x].startms);
    double newi = (v[x + 1].endi - v[x].starti) * d + v[x].starti;
    v[x].endi = v[x + 1].starti = newi;
}
#define MAXMS 99999999
int GetStartMS(int x,  std::vector<RGBData> &v) {
    if (x < v.size()) {
        return v[x].startms;
    }
    return MAXMS;
}
int GetEndMS(int x,  std::vector<RGBData> &v) {
    if (x < v.size()) {
        return v[x].endms;
    }
    return MAXMS;
}
void Resize(int x,
            std::vector<RGBData> &v, int startms) {
    while (x >= v.size()) {
        int i = v.size();
        v.push_back(RGBData());
        v[i].endms = MAXMS;
        v[i].startms = startms;
        v[i].starti = v[i].endi = 0;
        v[i].shimmer = false;
    }
}

void UnifyData(int x,
               std::vector<RGBData> &red,
               std::vector<RGBData> &green,
               std::vector<RGBData> &blue) {
    int min = std::min(GetStartMS(x, red), std::min(GetStartMS(x, green), GetStartMS(x ,blue)));
    Resize(x, red, min);
    Resize(x, green, min);
    Resize(x, blue, min);
    if (red[x].startms != min) {
        Insert(x, red, min);
    }
    if (green[x].startms != min) {
        Insert(x, green, min);
    }
    if (blue[x].startms != min) {
        Insert(x, blue, min);
    }
    min = std::min(GetEndMS(x, red), std::min(GetEndMS(x, green), GetEndMS(x ,blue)));
    if (min == MAXMS) {
        return;
    }
    if (red[x].endms != min) {
        Split(x, red, min);
    }
    if (green[x].endms != min) {
        Split(x, green, min);
    }
    if (blue[x].endms != min) {
        Split(x, blue, min);
    }

}
bool GetRGBEffectData(RGBData &red, RGBData &green, RGBData &blue, xlColor &sc, xlColor &ec) {

    sc.red = red.starti * 255 / 100;
    sc.green = green.starti * 255 / 100;
    sc.blue = blue.starti * 255 / 100;

    ec.red = red.endi * 255 / 100;
    ec.green = green.endi * 255 / 100;
    ec.blue = blue.endi * 255 / 100;

    return red.shimmer | blue.shimmer | green.shimmer;
}

void LoadRGBData(EffectLayer *layer, wxXmlNode *rchannel, wxXmlNode *gchannel, wxXmlNode *bchannel) {
    std::vector<RGBData> red, green, blue;
    while (rchannel != nullptr) {
        red.resize(red.size() + 1);
        FillData(rchannel, red[red.size() - 1]);
        rchannel = rchannel->GetNext();
    }
    while (gchannel != nullptr) {
        green.resize(green.size() + 1);
        FillData(gchannel, green[green.size() - 1]);
        gchannel = gchannel->GetNext();
    }
    while (bchannel != nullptr) {
        blue.resize(blue.size() + 1);
        FillData(bchannel, blue[blue.size() - 1]);
        bchannel = bchannel->GetNext();
    }
    //have the data, now need to split it so common start/end times
    for (int x = 0; x < red.size() || x < green.size() || x < blue.size(); x++) {
        UnifyData(x, red, green, blue);
    }

    int cwIndex = Effect::GetEffectIndex("Color Wash");
    int onIndex = Effect::GetEffectIndex("On");
    for (int x = 0; x < red.size() || x < green.size() || x < blue.size(); x++) {
        xlColor sc, ec;
        bool isShimmer = GetRGBEffectData(red[x], green[x], blue[x], sc, ec);

        int starttime = red[x].startms;
        int endtime = red[x].endms;

        if (ec == sc) {
            if (ec != xlBLACK) {
                wxString palette = "C_BUTTON_Palette1=" + sc + ",C_CHECKBOX_Palette1=1,"
                    + "C_BUTTON_Palette2=#000000,C_CHECKBOX_Palette2=0,"
                    + "C_CHECKBOX_Palette3=0,C_CHECKBOX_Palette4=0,C_CHECKBOX_Palette5=0,C_CHECKBOX_Palette6=0,"
                    + "C_SLIDER_Brightness=100,C_SLIDER_Contrast=0,C_SLIDER_SparkleFrequency=0";

                wxString settings = _("E_TEXTCTRL_Eff_On_End=100,E_TEXTCTRL_Eff_On_Start=100")
                    + ",E_TEXTCTRL_On_Cycles=1.00,T_CHECKBOX_LayerMorph=0,T_CHECKBOX_OverlayBkg=0,"
                    + "T_CHOICE_LayerMethod=Normal,T_SLIDER_EffectLayerMix=0,"
                    + "T_TEXTCTRL_Fadein=0.00,T_TEXTCTRL_Fadeout=0.00,E_CHECKBOX_On_Shimmer=" + (isShimmer ? "1" : "0");
                layer->AddEffect(0, onIndex, "On", settings, palette, starttime, endtime, false, false);
            }
        } else if (sc == xlBLACK) {
            wxString palette = "C_BUTTON_Palette1=" + ec + ",C_CHECKBOX_Palette1=1,"
                + "C_BUTTON_Palette2=#000000,C_CHECKBOX_Palette2=0,"
                + "C_CHECKBOX_Palette3=0,C_CHECKBOX_Palette4=0,C_CHECKBOX_Palette5=0,C_CHECKBOX_Palette6=0,"
                + "C_SLIDER_Brightness=100,C_SLIDER_Contrast=0,C_SLIDER_SparkleFrequency=0";
            wxString settings = _("E_TEXTCTRL_Eff_On_End=100,E_TEXTCTRL_Eff_On_Start=0")
                + ",E_TEXTCTRL_On_Cycles=1.00,T_CHECKBOX_LayerMorph=0,T_CHECKBOX_OverlayBkg=0,"
                + "T_CHOICE_LayerMethod=Normal,T_SLIDER_EffectLayerMix=0,"
                + "T_TEXTCTRL_Fadein=0.00,T_TEXTCTRL_Fadeout=0.00,E_CHECKBOX_On_Shimmer=" + (isShimmer ? "1" : "0");
            layer->AddEffect(0, onIndex, "On", settings, palette, starttime, endtime, false, false);
        } else if (ec == xlBLACK) {
            wxString palette = "C_BUTTON_Palette1=" + sc + ",C_CHECKBOX_Palette1=1,"
                + "C_BUTTON_Palette2=#000000,C_CHECKBOX_Palette2=0,"
                + "C_CHECKBOX_Palette3=0,C_CHECKBOX_Palette4=0,C_CHECKBOX_Palette5=0,C_CHECKBOX_Palette6=0,"
                + "C_SLIDER_Brightness=100,C_SLIDER_Contrast=0,C_SLIDER_SparkleFrequency=0";
            wxString settings = _("E_TEXTCTRL_Eff_On_End=0,E_TEXTCTRL_Eff_On_Start=100")
                + ",E_TEXTCTRL_On_Cycles=1.00,T_CHECKBOX_LayerMorph=0,T_CHECKBOX_OverlayBkg=0,"
                + "T_CHOICE_LayerMethod=Normal,T_SLIDER_EffectLayerMix=0,"
                + "T_TEXTCTRL_Fadein=0.00,T_TEXTCTRL_Fadeout=0.00,E_CHECKBOX_On_Shimmer=" + (isShimmer ? "1" : "0");
            layer->AddEffect(0, onIndex, "On", settings, palette, starttime, endtime, false, false);
        } else {
            wxString palette = "C_BUTTON_Palette1=" + sc + ",C_CHECKBOX_Palette1=1,"
                + "C_BUTTON_Palette2=" + ec + ",C_CHECKBOX_Palette2=1,"
                + "C_CHECKBOX_Palette3=0,C_CHECKBOX_Palette4=0,C_CHECKBOX_Palette5=0,C_CHECKBOX_Palette6=0,"
                + "C_SLIDER_Brightness=100,C_SLIDER_Contrast=0,C_SLIDER_SparkleFrequency=0";

            wxString settings = _("E_CHECKBOX_ColorWash_HFade=0,E_CHECKBOX_ColorWash_VFade=0,E_TEXTCTRL_ColorWash_Cycles=1.00,")
                + "E_TEXTCTRL_ColorWash_Cycles=1.00,E_CHECKBOX_ColorWash_CircularPalette=0,"
                + "T_CHECKBOX_LayerMorph=0,T_CHECKBOX_OverlayBkg=0,T_CHOICE_LayerMethod=Normal,"
                + "T_SLIDER_EffectLayerMix=0,T_TEXTCTRL_Fadein=0.00,T_TEXTCTRL_Fadeout=0.00"
                + ",E_CHECKBOX_ColorWash_Shimmer=" + (isShimmer ? "1" : "0");
            layer->AddEffect(0, cwIndex, "Color Wash", settings, palette, starttime, endtime, false, false);
        }
    }
}

void MapRGBEffects(EffectLayer *layer, wxXmlNode *rchannel, wxXmlNode *gchannel, wxXmlNode *bchannel) {
    wxXmlNode* re=rchannel->GetChildren();
    while (re != nullptr && "effect" != re->GetName()) re = re->GetNext();
    wxXmlNode* ge=gchannel->GetChildren();
    while (ge != nullptr && "effect" != ge->GetName()) ge = ge->GetNext();
    wxXmlNode* be=bchannel->GetChildren();
    while (be != nullptr && "effect" != be->GetName()) be = be->GetNext();
    LoadRGBData(layer, re, ge, be);
}
void MapOnEffects(EffectLayer *layer, wxXmlNode *channel, int chancountpernode, const wxColor &color) {
    wxString palette = _("C_BUTTON_Palette1=#FFFFFF,C_CHECKBOX_Palette1=1,")
        + "C_CHECKBOX_Palette2=0,C_CHECKBOX_Palette3=0,C_CHECKBOX_Palette4=0,C_CHECKBOX_Palette5=0,C_CHECKBOX_Palette6=0,"
        + "C_SLIDER_Brightness=100,C_SLIDER_Contrast=0,C_SLIDER_SparkleFrequency=0";
    if (chancountpernode > 1) {
        wxString c = wxString::Format("#%06lx",color.GetRGB());
        xlColor color(c);
        palette = "C_BUTTON_Palette1=" + color + ",C_CHECKBOX_Palette1=1,"
            + "C_CHECKBOX_Palette2=0,C_CHECKBOX_Palette3=0,C_CHECKBOX_Palette4=0,C_CHECKBOX_Palette5=0,C_CHECKBOX_Palette6=0,"
            + "C_SLIDER_Brightness=100,C_SLIDER_Contrast=0,C_SLIDER_SparkleFrequency=0";
    }
    int on_index = Effect::GetEffectIndex("On");

    for (wxXmlNode* ch=channel->GetChildren(); ch!=NULL; ch=ch->GetNext()) {
        if (ch->GetName() == "effect") {
            int starttime = (wxAtoi(ch->GetAttribute("startCentisecond"))) * 10;
            int endtime = (wxAtoi(ch->GetAttribute("endCentisecond"))) * 10;
            wxString intensity = ch->GetAttribute("intensity", "-1");
            wxString starti, endi;
            if (intensity == "-1") {
                starti = ch->GetAttribute("startIntensity");
                endi = ch->GetAttribute("endIntensity");
            } else {
                starti = endi = intensity;
            }
            wxString settings = "E_TEXTCTRL_Eff_On_End=" + endi +",E_TEXTCTRL_Eff_On_Start=" + starti
                + ",E_TEXTCTRL_On_Cycles=1.00,T_CHECKBOX_LayerMorph=0,T_CHECKBOX_OverlayBkg=0,"
                + "T_CHOICE_LayerMethod=Normal,T_SLIDER_EffectLayerMix=0,"
                + "T_TEXTCTRL_Fadein=0.00,T_TEXTCTRL_Fadeout=0.00,";
            if ("intensity" == ch->GetAttribute("type")) {
                settings += "E_CHECKBOX_On_Shimmer=0";
            } else {
                settings += "E_CHECKBOX_On_Shimmer=1";
            }
            layer->AddEffect(0, on_index, "On", settings, palette, starttime, endtime, false, false);
        }
    }
}
bool MapChannelInformation(EffectLayer *layer, wxXmlDocument &input_xml, const wxString &nm, const wxColor &color, const ModelClass &mc) {
    if ("" == nm) {
        return false;
    }
    wxXmlNode *channel = nullptr;
    wxXmlNode *rchannel = nullptr;
    wxXmlNode *gchannel = nullptr;
    wxXmlNode *bchannel = nullptr;
    for(wxXmlNode* e=input_xml.GetRoot()->GetChildren(); e!=NULL; e=e->GetNext()) {
        if (e->GetName() == "channels"){
            for (wxXmlNode* chan=e->GetChildren(); chan!=NULL; chan=chan->GetNext()) {
                if ((chan->GetName() == "channel" || chan->GetName() == "rgbChannel")
                    && nm == chan->GetAttribute("name")) {
                    channel = chan;
                    if (chan->GetName() == "rgbChannel"
                        && !findRGB(e, chan, rchannel, gchannel, bchannel)) {
                        return false;
                    }
                    break;
                }
            }
        }
    }
    if (channel == nullptr) {
        return false;
    }
    if (channel->GetName() == "rgbChannel") {
        MapRGBEffects(layer, rchannel, gchannel, bchannel);
    } else {
        MapOnEffects(layer, channel, mc.GetChanCountPerNode(), color);
    }
    return true;
}

bool xLightsFrame::ImportLMS(wxXmlDocument &input_xml)
{
    LMSImportChannelMapDialog dlg(this);
    dlg.mSequenceElements = &mSequenceElements;
    dlg.xlights = this;

    for(wxXmlNode* e=input_xml.GetRoot()->GetChildren(); e!=NULL; e=e->GetNext()) {
        if (e->GetName() == "channels"){
            for (wxXmlNode* chan=e->GetChildren(); chan!=NULL; chan=chan->GetNext()) {
                if (chan->GetName() == "channel" || chan->GetName() == "rgbChannel") {
                    wxString name = chan->GetAttribute("name");
                    if (chan->GetName() == "rgbChannel") {
                        dlg.channelColors[name] = xlBLACK;
                    } else {
                        wxString color = chan->GetAttribute("color");
                        dlg.channelColors[name] = GetColor(color);
                    }

                    dlg.channelNames.push_back(name);
                    if (chan->GetName() == "rgbChannel") {
                        int idxDP = name.Find("-P");
                        int idxUP = name.Find(" P");
                        int idxSP = name.Find(" p");
                        if (idxUP > idxSP) {
                            idxSP = idxUP;
                        }
                        if (idxDP > idxSP) {
                            idxSP = idxDP;
                        }
                        if (idxSP != wxNOT_FOUND) {
                            int i = wxAtoi(name.SubString(idxSP + 2, name.size()));
                            if (i > 0
                                && (dlg.ccrNames.size() == 0 || dlg.ccrNames.back() != name.SubString(0, idxSP - 1))) {
                                dlg.ccrNames.push_back(name.SubString(0, idxSP - 1));
                            }
                        }
                    }
                }
            }
        }
    }
    dlg.channelNames.Sort();
    dlg.channelNames.Insert("", 0);
    dlg.ccrNames.Sort();
    dlg.ccrNames.Insert("", 0);

    dlg.Init();

    if (dlg.ShowModal() != wxID_OK) {
        return false;
    }

    int row = 0;
    for (int m = 0; m < dlg.modelNames.size(); m++) {
        wxString modelName = dlg.modelNames[m];
        ModelClass *mc = GetModelClass(modelName);
        Element * model = nullptr;
        for (int i=0;i<mSequenceElements.GetElementCount();i++) {
            if (mSequenceElements.GetElement(i)->GetType() == "model"
                && modelName == mSequenceElements.GetElement(i)->GetName()) {
                model = mSequenceElements.GetElement(i);
            }
        }
        MapChannelInformation(model->GetEffectLayer(0), input_xml,
                              dlg.ChannelMapGrid->GetCellValue(row, 3),
                              dlg.ChannelMapGrid->GetCellBackgroundColour(row, 4), *mc);
        row++;

        for (int str = 0; str < mc->GetNumStrands(); str++) {
            StrandLayer *sl = model->GetStrandLayer(str);

            if ("" != dlg.ChannelMapGrid->GetCellValue(row, 3)) {
                if (!dlg.MapByStrand->GetValue()) {
                    MapChannelInformation(sl,
                                      input_xml,
                                      dlg.ChannelMapGrid->GetCellValue(row, 3),
                                      dlg.ChannelMapGrid->GetCellBackgroundColour(row, 4), *mc);
                } else {
                    wxString ccrName = dlg.ChannelMapGrid->GetCellValue(row, 3);
                    for (int n = 0; n < sl->GetNodeLayerCount(); n++) {
                        EffectLayer *layer = sl->GetNodeLayer(n);
                        wxString nm = ccrName + wxString::Format("-P%02d", (n + 1));
                        if (dlg.channelNames.Index(nm) == wxNOT_FOUND) {
                            nm = ccrName + wxString::Format(" p%02d", (n + 1));
                        }
                        if (dlg.channelNames.Index(nm) == wxNOT_FOUND) {
                            nm = ccrName + wxString::Format("-P%d", (n + 1));
                        }
                        if (dlg.channelNames.Index(nm) == wxNOT_FOUND) {
                            nm = ccrName + wxString::Format(" p%d", (n + 1));
                        }
                        if (dlg.channelNames.Index(nm) == wxNOT_FOUND) {
                            nm = ccrName + wxString::Format(" P %02d", (n + 1));
                        }
                        MapChannelInformation(layer,
                                              input_xml,
                                              nm,
                                              dlg.ChannelMapGrid->GetCellBackgroundColour(row, 4),
                                              *mc);
                    }
                }
            }
            row++;
            if (!dlg.MapByStrand->GetValue()) {
                for (int n = 0; n < mc->GetStrandLength(str); n++) {
                    if ("" != dlg.ChannelMapGrid->GetCellValue(row, 3)) {
                        MapChannelInformation(sl->GetNodeLayer(n),
                                              input_xml,
                                              dlg.ChannelMapGrid->GetCellValue(row, 3),
                                              dlg.ChannelMapGrid->GetCellBackgroundColour(row, 4),
                                              *mc);
                    }
                    row++;
                }
            }
        }

    }

    return true;
}
unsigned char ChannelBlend(unsigned char c1, unsigned char  c2, double ratio)
{
    return c1 + floor(ratio*(c2-c1)+0.5);
}

class ImageInfo {
public:
    int xoffset;
    int yoffset;
    int width;
    int height;
    wxString imageName;

    void Set(int x, int y, int w, int h, const wxString &n) {
        xoffset = x;
        yoffset = y;
        width = w;
        height = h;
        imageName = n;
    }
};

wxString CreateSceneImage(const wxString &imagePfx, const wxString &postFix,
                          wxXmlNode *element, int numCols,
                          int numRows, bool reverse, const xlColor &color, int y_offset) {
    wxImage i;
    i.Create(numCols, numRows);
    i.InitAlpha();
    for (int x = 0; x < numCols; x++)  {
        for (int y = 0; y < numRows; y++) {
            i.SetAlpha(x, y, wxALPHA_TRANSPARENT);
        }
    }
    for(wxXmlNode* e=element->GetChildren(); e!=NULL; e=e->GetNext()) {
        if (e->GetName() == "element") {
            int x = wxAtoi(e->GetAttribute("ribbonIndex"));
            int y = wxAtoi(e->GetAttribute("pixelIndex")) - y_offset;
            if (x < numCols && y >=0 && y < numRows) {
                i.SetRGB(x, y, color.Red(), color.Green(), color.Blue());
                i.SetAlpha(x, y, wxALPHA_OPAQUE);
            }
        }
    }
    wxString name = imagePfx + "_s" + element->GetAttribute("savedIndex") + postFix + ".png";
    i.SaveFile(name);
    return name;
}
bool IsPartOfModel(wxXmlNode *element, int num_rows, int num_columns, bool &isFull, wxRect &rect) {
    std::vector< std::vector<bool> > data(num_columns, std::vector<bool>(num_rows));
    int maxCol = -1;
    int maxRow = -1;
    int minCol = 9999999;
    int minRow = 9999999;
    isFull = true;
    for(wxXmlNode* e=element->GetChildren(); e!=NULL; e=e->GetNext()) {
        if (e->GetName() == "element") {
            int x = wxAtoi(e->GetAttribute("ribbonIndex"));
            int y = wxAtoi(e->GetAttribute("pixelIndex"));
            if (x < num_columns) {
                data[x][y] = true;
                if (x > maxCol) maxCol = x;
                if (x < minCol) minCol = x;
                if (y > maxRow) maxRow = y;
                if (y < minRow) minRow = y;
            } else {
                return false;
            }
        }
    }
    isFull = minCol == 0 && minRow == 0 && maxRow == (num_rows - 1) && maxCol == (num_columns - 1);
    bool isRect = true;
    for (int x = minCol; x <= maxCol; x++) {
        for (int y = minRow; y <= maxRow; y++) {
            if (!data[x][y]) {
                isFull = false;
                isRect = false;
            }
        }
    }
    if (isRect) {
        rect.x = minCol;
        rect.y = minRow;
        rect.width = maxCol;
        rect.height = maxRow;
    } else {
        rect.x = -1;
        rect.y = -1;
    }
    return true;
}

bool xLightsFrame::ImportSuperStar(Element *model, wxXmlDocument &input_xml, int x_size, int y_size, int x_offset, int y_offset, bool flip_y)
{
    double num_rows = 1.0;
    double num_columns = 1.0;
    bool reverse_rows = false;
    bool layout_defined = false;
    wxXmlNode* input_root=input_xml.GetRoot();
    int morph_index = Effect::GetEffectIndex("Morph");
    int galaxy_index = Effect::GetEffectIndex("Galaxy");
    int shockwave_index = Effect::GetEffectIndex("Shockwave");
    int fan_index = Effect::GetEffectIndex("Fan");
    EffectLayer* layer = model->AddEffectLayer();
    std::map<int, ImageInfo> imageInfo;
    wxString imagePfx;
    std::vector<bool> reserved;
    for(wxXmlNode* e=input_root->GetChildren(); e!=NULL; e=e->GetNext()) {
        if ("imageActions" == e->GetName()) {
            for(wxXmlNode* element=e->GetChildren(); element!=NULL; element=element->GetNext()) {
                if ("imageAction" == element->GetName()) {
                    int layer_index = wxAtoi(element->GetAttribute("layer"));
                    if (layer_index > 0) layer_index--;
                    if (layer_index >= reserved.size()) {
                        reserved.resize(layer_index + 1, false);
                    }
                    reserved[layer_index] = true;
                }
            }
        }
    }
    for(wxXmlNode* e=input_root->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (e->GetName() == "layouts")
        {
            wxXmlNode* element=e->GetChildren();
            wxString attr;
            element->GetAttribute("visualizationMode", &attr);
            if( attr == "false" )
            {
                element->GetAttribute("nbrOfRibbons", &attr);
                attr.ToDouble(&num_columns);
                num_rows = 50.0;
                element->GetAttribute("ribbonLength", &attr);
                if( attr == "half" )
                {
                    num_rows /= 2.0;
                    num_columns *= 2.0;
                }
            }
            else
            {
                num_rows = (double)y_size;
                num_columns = (double)x_size;
            }
            element->GetAttribute("controllerLocation", &attr);
            if( attr == "bottom" || flip_y )
            {
                reverse_rows = true;
            }
            layout_defined = true;
        }
        if (e->GetName() == "morphs")
        {
            if( !layout_defined )
            {
                wxMessageBox("The layouts section was not found in the SuperStar file!");
                return false;
            }
            for(wxXmlNode* element=e->GetChildren(); element!=NULL; element=element->GetNext() )
            {
                int layer_index;
                double layer_val;
                wxString name_attr;
                wxString acceleration;
                wxString state1_time, state2_time, ramp_time_ext, attr;
                int start_time, end_time, ramp_time;
                element->GetAttribute("name", &name_attr);
                element->GetAttribute("acceleration", &acceleration);
                element->GetAttribute("layer", &attr);
                attr.ToDouble(&layer_val);
                layer_index = (int)layer_val;
                wxXmlNode* state1=element->GetChildren();
                wxXmlNode* state2=state1->GetNext();
                wxXmlNode* ramp=state2->GetNext();
                state1->GetAttribute("time", &state1_time);
                state2->GetAttribute("time", &state2_time);
                ramp->GetAttribute("timeExt", &ramp_time_ext);
                start_time = wxAtoi(state1_time) * 10;
                end_time = wxAtoi(state2_time) * 10;
                ramp_time = wxAtoi(ramp_time_ext) * 10;
                end_time += ramp_time;
                double head_duration = (1.0 - (double)ramp_time/((double)end_time-(double)start_time)) * 100.0;
                wxString settings = "E_CHECKBOX_Morph_End_Link=0,E_CHECKBOX_Morph_Start_Link=0,E_CHECKBOX_ShowHeadAtStart=0,E_NOTEBOOK_Morph=Start,E_SLIDER_MorphAccel=0,E_SLIDER_Morph_Repeat_Count=0,E_SLIDER_Morph_Repeat_Skip=1,E_SLIDER_Morph_Stagger=0";
                settings += acceleration + ",";
                wxString duration = wxString::Format("E_SLIDER_MorphDuration=%d,",(int)head_duration);
                settings += duration;
                state2->GetAttribute("trailLen", &attr);
                settings += "E_SLIDER_MorphEndLength=" + attr + ",";
                state1->GetAttribute("trailLen", &attr);
                settings += "E_SLIDER_MorphStartLength=" + attr + ",";
                state2->GetAttribute("x1", &attr);
                if( !CalcPercentage(attr, num_columns, false, x_offset) ) continue;
                settings += "E_SLIDER_Morph_End_X1=" + attr + ",";
                state2->GetAttribute("x2", &attr);
                if( !CalcPercentage(attr, num_columns, false, x_offset) ) continue;
                settings += "E_SLIDER_Morph_End_X2=" + attr + ",";
                state2->GetAttribute("y1", &attr);
                if( !CalcPercentage(attr, num_rows, reverse_rows, y_offset) ) continue;
                settings += "E_SLIDER_Morph_End_Y1=" + attr + ",";
                state2->GetAttribute("y2", &attr);
                if( !CalcPercentage(attr, num_rows, reverse_rows, y_offset) ) continue;
                settings += "E_SLIDER_Morph_End_Y2=" + attr + ",";
                state1->GetAttribute("x1", &attr);
                if( !CalcPercentage(attr, num_columns, false, x_offset) ) continue;
                settings += "E_SLIDER_Morph_Start_X1=" + attr + ",";
                state1->GetAttribute("x2", &attr);
                if( !CalcPercentage(attr, num_columns, false, x_offset) ) continue;
                settings += "E_SLIDER_Morph_Start_X2=" + attr + ",";
                state1->GetAttribute("y1", &attr);
                if( !CalcPercentage(attr, num_rows, reverse_rows, y_offset) ) continue;
                settings += "E_SLIDER_Morph_Start_Y1=" + attr + ",";
                state1->GetAttribute("y2", &attr);
                if( !CalcPercentage(attr, num_rows, reverse_rows, y_offset) ) continue;
                settings += "E_SLIDER_Morph_Start_Y2=" + attr + ",";
                wxString sRed, sGreen, sBlue,color;
                state1->GetAttribute("red", &sRed);
                state1->GetAttribute("green", &sGreen);
                state1->GetAttribute("blue", &sBlue);
                color = GetColorString(sRed, sGreen, sBlue);
                wxString palette = "C_BUTTON_Palette1=" + color + ",";
                state2->GetAttribute("red", &sRed);
                state2->GetAttribute("green", &sGreen);
                state2->GetAttribute("blue", &sBlue);
                color = GetColorString(sRed, sGreen, sBlue);
                palette += "C_BUTTON_Palette2=" + color + ",";
                ramp->GetAttribute("red1", &sRed);
                ramp->GetAttribute("green1", &sGreen);
                ramp->GetAttribute("blue1", &sBlue);
                color = GetColorString(sRed, sGreen, sBlue);
                palette += "C_BUTTON_Palette3=" + color + ",";
                ramp->GetAttribute("red2", &sRed);
                ramp->GetAttribute("green2", &sGreen);
                ramp->GetAttribute("blue2", &sBlue);
                color = GetColorString(sRed, sGreen, sBlue);
                palette += "C_BUTTON_Palette4=" + color + ",";
                palette += "C_BUTTON_Palette5=#FFFFFF,C_BUTTON_Palette6=#000000,C_CHECKBOX_Palette1=1,C_CHECKBOX_Palette2=1,C_CHECKBOX_Palette3=1,C_CHECKBOX_Palette4=1,";
                palette += "C_CHECKBOX_Palette5=0,C_CHECKBOX_Palette6=0,C_SLIDER_Brightness=100,C_SLIDER_Contrast=0,C_SLIDER_SparkleFrequency=0";
                settings += "T_CHECKBOX_LayerMorph=0,T_CHECKBOX_OverlayBkg=0,";
                if( color == xlBLACK ) {
                    settings += "T_CHOICE_LayerMethod=Normal,";
                }
                else {
                    settings += "T_CHOICE_LayerMethod=1 reveals 2,";
                }
                settings += "T_SLIDER_EffectLayerMix=0,T_TEXTCTRL_Fadein=0.00,T_TEXTCTRL_Fadeout=0.00";
                while( model->GetEffectLayerCount() < layer_index )
                {
                    model->AddEffectLayer();
                }
                layer = FindOpenLayer(model, layer_index, start_time, end_time, reserved);
                layer->AddEffect(0, morph_index, "Morph", settings, palette, start_time, end_time, false, false);
            }
        } else if ("images" == e->GetName()) {
            for(wxXmlNode* element=e->GetChildren(); element!=NULL; element=element->GetNext()) {
                if ("image" == element->GetName()) {
                    for(wxXmlNode* i=element->GetChildren(); i!=NULL; i=i->GetNext()) {
                        if ("pixe" == i->GetName()){
                            wxString data = i->GetAttribute("s");
                            int w = wxAtoi(element->GetAttribute("width"));
                            int h = wxAtoi(element->GetAttribute("height"));


                            int idx = wxAtoi(element->GetAttribute("savedIndex"));
                            int xOffset =  wxAtoi(element->GetAttribute("xOffset"));
                            int yOffset =  wxAtoi(element->GetAttribute("yOffset"));
                            unsigned char *bytes = (unsigned char *)malloc(w*h*3);
                            unsigned char *alpha = (unsigned char *)malloc(w*h);
                            int cnt = 0;
                            int p = 0;
                            wxStringTokenizer tokenizer(data, ",");
                            while (tokenizer.HasMoreTokens()) {
                                unsigned int i = wxAtoi(tokenizer.GetNextToken());
                                unsigned int v = (i >> 16) & 0xff;
                                v *= 255;
                                v /= 100;
                                bytes[cnt] = v;
                                v = (i >> 8) & 0xff;
                                v *= 255;
                                v /= 100;
                                bytes[cnt + 1] = v;
                                v = i & 0xff;
                                v *= 255;
                                v /= 100;
                                bytes[cnt + 2] = v;

                                alpha[p] = wxALPHA_OPAQUE;
                                if (i == 0) {
                                    alpha[p] = wxALPHA_TRANSPARENT;
                                }
                                p++;
                                cnt += 3;
                            }

                            wxImage image(w, h, bytes, alpha);
                            if ("" == imagePfx) {
                                wxFileDialog fd(this,
                                                "Choose location and base name for image files",
                                                showDirectory,
                                                wxEmptyString,
                                                wxFileSelectorDefaultWildcardStr,
                                                wxFD_SAVE);
                                while (fd.ShowModal() == wxID_CANCEL || fd.GetFilename() == "") {
                                }
                                imagePfx = fd.GetPath();
                            }
                            wxString fname = imagePfx + "_" + wxString::Format("%d.png", idx);
                            imageInfo[idx].Set(xOffset, yOffset, w, h, fname);
                            image.SaveFile(fname);
                        }
                    }
                }
            }
        } else if ("flowys" == e->GetName()) {
            for(wxXmlNode* element=e->GetChildren(); element!=NULL; element=element->GetNext()) {
                if ("flowy" == element->GetName()) {
                    wxString centerX, centerY;
                    int startms = wxAtoi(element->GetAttribute("startTime")) * 10;
                    int endms = wxAtoi(element->GetAttribute("endTime")) * 10;
                    wxString type = element->GetAttribute("flowyType");
                    wxString color_string = element->GetAttribute("Colors");
                    wxString sRed, sGreen, sBlue, color;
                    wxString palette = "C_BUTTON_Palette1=" + color + ",";
                    int cnt = 1;
                    wxStringTokenizer tokenizer(color_string, " ");
                    while (tokenizer.HasMoreTokens() && cnt <=6) {
                        wxStringTokenizer tokenizer2(tokenizer.GetNextToken(), ",");
                        sRed = tokenizer2.GetNextToken();
                        sGreen = tokenizer2.GetNextToken();
                        sBlue = tokenizer2.GetNextToken();
                        color = GetColorString(sRed, sGreen, sBlue);
                        if( cnt > 1 ) {
                            palette += ",";
                        }
                        palette += "C_BUTTON_Palette" + wxString::Format("%d", cnt) + "=" + color;
                        palette += ",C_CHECKBOX_Palette" + wxString::Format("%d", cnt) + "=1";
                        cnt++;
                    }
                    while (cnt<=6) {
                        if( cnt > 1 ) {
                            palette += ",";
                        }
                        palette += "C_BUTTON_Palette" + wxString::Format("%d", cnt) + "=#000000";
                        palette += ",C_CHECKBOX_Palette" + wxString::Format("%d", cnt) + "=0";
                        cnt++;
                    }
                    palette += ",C_SLIDER_Brightness=100,C_SLIDER_Contrast=0,C_SLIDER_SparkleFrequency=0";

                    int layer_index = wxAtoi(element->GetAttribute("layer"));
                    int acceleration = wxAtoi(element->GetAttribute("acceleration"));
                    element->GetAttribute("centerX", &centerX);
                    if( !CalcPercentage(centerX, num_columns, false, x_offset) ) continue;
                    element->GetAttribute("centerY", &centerY);
                    if( !CalcPercentage(centerY, num_rows, reverse_rows, y_offset) ) continue;
                    int startAngle = wxAtoi(element->GetAttribute("startAngle"));
                    int endAngle = wxAtoi(element->GetAttribute("endAngle"));
                    int revolutions = std::abs(endAngle-startAngle);
                    if( revolutions == 0 ) revolutions = 3;  // algorithm needs non-zero value until we figure out better way to draw effect
                    int startRadius = wxAtoi(element->GetAttribute("startRadius"));
                    int endRadius = wxAtoi(element->GetAttribute("endRadius"));
                    layer = FindOpenLayer(model, layer_index, startms, endms, reserved);
                    if( type == "Spiral" )
                    {
                        int tailms = wxAtoi(element->GetAttribute("tailTimeLength")) * 10;
                        endms += tailms;
                        double duration = (1.0 - (double)tailms/((double)endms-(double)startms)) * 100.0;
                        int startWidth = wxAtoi(element->GetAttribute("startDotSize"));
                        int endWidth = wxAtoi(element->GetAttribute("endDotSize"));
                        wxString settings = "E_CHECKBOX_Galaxy_Reverse=" + wxString::Format("%d", startAngle < endAngle)
                                            + ",E_CHECKBOX_Galaxy_Blend_Edges=1"
                                            + ",E_CHECKBOX_Galaxy_Inward=1"
                                            + ",E_NOTEBOOK_Galaxy=Start,E_SLIDER_Galaxy_Accel=" + wxString::Format("%d", acceleration)
                                            + ",E_SLIDER_Galaxy_CenterX=" + centerX
                                            + ",E_SLIDER_Galaxy_CenterY=" + centerY
                                            + ",E_SLIDER_Galaxy_Duration=" + wxString::Format("%d", (int)duration)
                                            + ",E_SLIDER_Galaxy_End_Radius=" + wxString::Format("%d", endRadius)
                                            + ",E_SLIDER_Galaxy_End_Width=" + wxString::Format("%d", endWidth)
                                            + ",E_SLIDER_Galaxy_Revolutions=" + wxString::Format("%d", revolutions)
                                            + ",E_SLIDER_Galaxy_Start_Angle=" + wxString::Format("%d", startAngle)
                                            + ",E_SLIDER_Galaxy_Start_Radius=" + wxString::Format("%d", startRadius)
                                            + ",E_SLIDER_Galaxy_Start_Width=" + wxString::Format("%d", startWidth)
                                            + ",T_CHECKBOX_LayerMorph=0,T_CHECKBOX_OverlayBkg=0,"
                                            + "T_CHOICE_LayerMethod=Normal,T_SLIDER_EffectLayerMix=0,T_TEXTCTRL_Fadein=0.00"
                                            + ",T_TEXTCTRL_Fadeout=0.00";
                        layer->AddEffect(0, galaxy_index, "Galaxy", settings, palette, startms, endms, false, false);
                    }
                    else if( type == "Shockwave" )
                    {
                        int startWidth = wxAtoi(element->GetAttribute("headWidth"));
                        int endWidth = wxAtoi(element->GetAttribute("tailWidth"));
                        wxString settings = "E_CHECKBOX_Shockwave_Blend_Edges=1"
                                            + _(",E_NOTEBOOK_Shockwave=Position,E_SLIDER_Shockwave_Accel=") + wxString::Format("%d", acceleration)
                                            + ",E_SLIDER_Shockwave_CenterX=" + centerX
                                            + ",E_SLIDER_Shockwave_CenterY=" + centerY
                                            + ",E_SLIDER_Shockwave_End_Radius=" + wxString::Format("%d", endRadius)
                                            + ",E_SLIDER_Shockwave_End_Width=" + wxString::Format("%d", endWidth)
                                            + ",E_SLIDER_Shockwave_Start_Radius=" + wxString::Format("%d", startRadius)
                                            + ",E_SLIDER_Shockwave_Start_Width=" + wxString::Format("%d", startWidth)
                                            + ",T_CHECKBOX_LayerMorph=0,T_CHECKBOX_OverlayBkg=0,"
                                            + "T_CHOICE_LayerMethod=Normal,T_SLIDER_EffectLayerMix=0,T_TEXTCTRL_Fadein=0.00"
                                            + ",T_TEXTCTRL_Fadeout=0.00";
                        layer->AddEffect(0, shockwave_index, "Shockwave", settings, palette, startms, endms, false, false);
                    }
                    else if( type == "Fan" )
                    {
                        int revolutionsPerSecond = wxAtoi(element->GetAttribute("revolutionsPerSecond"));
                        int blades = wxAtoi(element->GetAttribute("blades"));
                        int blade_width = wxAtoi(element->GetAttribute("width"));
                        int elementAngle = wxAtoi(element->GetAttribute("elementAngle"));
                        int elementStepAngle = wxAtoi(element->GetAttribute("elementStepAngle"));
                        int numElements = (int)(((360.0/(double)blades)*((double)blade_width/100.0))/(double)elementStepAngle);
                        numElements = std::max(1, numElements);
                        numElements = std::min(numElements, 4);
                        blades = std::max(1, blades);
                        blades = std::min(blades, 16);
                        wxString settings = "E_CHECKBOX_Fan_Reverse=" + wxString::Format("%d", startAngle > endAngle)
                                            + ",E_CHECKBOX_Fan_Blend_Edges=1"
                                            + ",E_NOTEBOOK_Fan=Position,E_SLIDER_Fan_Accel=" + wxString::Format("%d", acceleration)
                                            + ",E_SLIDER_Fan_Blade_Angle=" + wxString::Format("%d", elementAngle)
                                            + ",E_SLIDER_Fan_Blade_Width=" + wxString::Format("%d", blade_width)
                                            + ",E_SLIDER_Fan_CenterX=" + centerX
                                            + ",E_SLIDER_Fan_CenterY=" + centerY
                                            + ",E_SLIDER_Fan_Duration=100"
                                            + ",E_SLIDER_Fan_Element_Width=" + wxString::Format("%d", 100)
                                            + ",E_SLIDER_Fan_Num_Blades=" + wxString::Format("%d", blades)
                                            + ",E_SLIDER_Fan_Num_Elements=" + wxString::Format("%d", numElements)
                                            + ",E_SLIDER_Fan_End_Radius=" + wxString::Format("%d", endRadius)
                                            + ",E_SLIDER_Fan_Revolutions=" + wxString::Format("%d", (int)((double)revolutionsPerSecond*((double)(endms-startms)/1000.0)*3.6))
                                            + ",E_SLIDER_Fan_Start_Angle=" + wxString::Format("%d", startAngle)
                                            + ",E_SLIDER_Fan_Start_Radius=" + wxString::Format("%d", startRadius)
                                            + ",T_CHECKBOX_LayerMorph=0,T_CHECKBOX_OverlayBkg=0,"
                                            + "T_CHOICE_LayerMethod=Normal,T_SLIDER_EffectLayerMix=0,T_TEXTCTRL_Fadein=0.00"
                                            + ",T_TEXTCTRL_Fadeout=0.00";
                        layer->AddEffect(0, fan_index, "Fan", settings, palette, startms, endms, false, false);
                    }
                }
            }
        } else if ("scenes" == e->GetName()) {
            for(wxXmlNode* element=e->GetChildren(); element!=NULL; element=element->GetNext()) {
                if ("scene" == element->GetName()) {
                    wxString startms = element->GetAttribute("startCentisecond") + "0";
                    wxString endms = element->GetAttribute("endCentisecond") + "0";
                    wxString type = element->GetAttribute("type");
                    int layer_index = wxAtoi(element->GetAttribute("layer"));
                    xlColor startc = GetColor(element->GetAttribute("red1"),
                                             element->GetAttribute("green1"),
                                             element->GetAttribute("blue1"));
                    xlColor endc = GetColor(element->GetAttribute("red2"),
                                             element->GetAttribute("green2"),
                                             element->GetAttribute("blue2"));
                    while( model->GetEffectLayerCount() < layer_index ) {
                        model->AddEffectLayer();
                    }

                    int start_time = wxAtoi(startms);
                    int end_time = wxAtoi(endms);
                    layer = FindOpenLayer(model, layer_index, start_time, end_time, reserved);
                    if ("" == imagePfx) {
                        wxFileDialog fd(this,
                                        "Choose location and base name for image files",
                                        showDirectory,
                                        wxEmptyString,
                                        wxFileSelectorDefaultWildcardStr,
                                        wxFD_SAVE);
                        while (fd.ShowModal() == wxID_CANCEL || fd.GetFilename() == "") {
                        }
                        imagePfx = fd.GetPath();
                    }

                    wxString ru = "0.0";
                    wxString rd = "0.0";
                    wxString imageName;
                    bool isFull = false;
                    wxRect rect;

                    bool isPartOfModel = IsPartOfModel(element, num_rows, num_columns, isFull, rect);

                    if (isPartOfModel && isFull) {
                        //Every pixel in the model is specified, we can use a color wash or on instead of images


                        wxString palette = _("C_BUTTON_Palette1=") + startc + ",C_CHECKBOX_Palette1=1,C_BUTTON_Palette2=" + endc
                            + ",C_CHECKBOX_Palette2=1,C_CHECKBOX_Palette3=0,C_CHECKBOX_Palette4=0,C_CHECKBOX_Palette5=0,C_CHECKBOX_Palette6=0,"
                            + "C_SLIDER_Brightness=100,C_SLIDER_Contrast=0,C_SLIDER_SparkleFrequency=0";


                        if (startc == endc) {
                            wxString settings = _("E_TEXTCTRL_Eff_On_End=100,E_TEXTCTRL_Eff_On_Start=100")
                                + ",E_TEXTCTRL_On_Cycles=1.0,T_CHECKBOX_LayerMorph=0,T_CHECKBOX_OverlayBkg=0,"
                                + "T_CHOICE_LayerMethod=Normal,T_SLIDER_EffectLayerMix=0,"
                                + "T_TEXTCTRL_Fadein=0.00,T_TEXTCTRL_Fadeout=0.00";
                            layer->AddEffect(0, "On", settings, palette, start_time, end_time, false, false);
                        } else if (startc == xlBLACK) {
                            wxString palette = _("C_BUTTON_Palette1=") + endc + ",C_CHECKBOX_Palette1=1,C_BUTTON_Palette2=" + startc
                                + ",C_CHECKBOX_Palette2=1,C_CHECKBOX_Palette3=0,C_CHECKBOX_Palette4=0,C_CHECKBOX_Palette5=0,C_CHECKBOX_Palette6=0,"
                                + "C_SLIDER_Brightness=100,C_SLIDER_Contrast=0,C_SLIDER_SparkleFrequency=0";
                            wxString settings = _("E_TEXTCTRL_Eff_On_End=100,E_TEXTCTRL_Eff_On_Start=0")
                                + ",E_TEXTCTRL_On_Cycles=1.0,T_CHECKBOX_LayerMorph=0,T_CHECKBOX_OverlayBkg=0,"
                                + "T_CHOICE_LayerMethod=Normal,T_SLIDER_EffectLayerMix=0,"
                                + "T_TEXTCTRL_Fadein=0.00,T_TEXTCTRL_Fadeout=0.00";
                            layer->AddEffect(0, "On", settings, palette, start_time, end_time, false, false);
                        } else if (endc == xlBLACK) {
                            wxString settings = _("E_TEXTCTRL_Eff_On_End=0,E_TEXTCTRL_Eff_On_Start=100")
                                + ",E_TEXTCTRL_On_Cycles=1.0,T_CHECKBOX_LayerMorph=0,T_CHECKBOX_OverlayBkg=0,"
                                + "T_CHOICE_LayerMethod=Normal,T_SLIDER_EffectLayerMix=0,"
                                + "T_TEXTCTRL_Fadein=0.00,T_TEXTCTRL_Fadeout=0.00";
                            layer->AddEffect(0, "On", settings, palette, start_time, end_time, false, false);
                        } else {
                            wxString settings = _("T_CHOICE_LayerMethod=Normal,T_SLIDER_EffectLayerMix=0,")
                                + "T_TEXTCTRL_Fadein=0.00,T_TEXTCTRL_Fadeout=0.00,E_CHECKBOX_ColorWash_HFade=0,"
                                + "E_TEXTCTRL_ColorWash_Cycles=1.00,E_CHECKBOX_ColorWash_CircularPalette=0,"
                                + "E_CHECKBOX_ColorWash_VFade=0,E_CHECKBOX_ColorWash_EntireModel=1";
                            layer->AddEffect(0, "Color Wash", settings, palette, start_time, end_time, false, false);
                        }
                    } else if (isPartOfModel && rect.x != -1) {
                        //forms a simple rectangle, we can use a ColorWash affect for this with a partial rectangle
                        wxString palette = _("C_BUTTON_Palette1=") + startc + ",C_CHECKBOX_Palette1=1,C_BUTTON_Palette2=" + endc
                            + ",C_CHECKBOX_Palette2=1,C_CHECKBOX_Palette3=0,C_CHECKBOX_Palette4=0,C_CHECKBOX_Palette5=0,C_CHECKBOX_Palette6=0,"
                            + "C_SLIDER_Brightness=100,C_SLIDER_Contrast=0,C_SLIDER_SparkleFrequency=0";

                        wxString settings = "";
                        wxString val = wxString::Format("%d", rect.x);
                        if( !CalcPercentage(val, num_columns, false, x_offset) ) continue;
                        settings += ",E_SLIDER_ColorWash_X1=" + val;
                        val = wxString::Format("%d", rect.width);
                        if( !CalcPercentage(val, num_columns, false, x_offset) ) continue;
                        settings += ",E_SLIDER_ColorWash_X2=" + val;
                        val = wxString::Format("%d", rect.y);
                        if( !CalcPercentage(val, num_rows, true, y_offset) ) continue;
                        settings += ",E_SLIDER_ColorWash_Y1=" + val;
                        val = wxString::Format("%d", rect.height);
                        if( !CalcPercentage(val, num_rows, true, y_offset) ) continue;
                        settings += ",E_SLIDER_ColorWash_Y2=" + val;

                        settings = _("T_CHOICE_LayerMethod=Normal,T_SLIDER_EffectLayerMix=0,")
                            + "E_TEXTCTRL_ColorWash_Cycles=1.00,E_CHECKBOX_ColorWash_CircularPalette=0,"
                            + "T_TEXTCTRL_Fadein=0.00,T_TEXTCTRL_Fadeout=0.00,E_CHECKBOX_ColorWash_HFade=0,E_CHECKBOX_ColorWash_VFade=0,"
                            + "E_CHECKBOX_ColorWash_EntireModel=0" + settings;


                        layer->AddEffect(0, "Color Wash", settings, palette, start_time, end_time, false, false);
                    } else if (isPartOfModel) {
                        if (startc == xlBLACK || endc == xlBLACK || endc == startc) {
                            imageName = CreateSceneImage(imagePfx, "", element, num_columns, num_rows, reverse_rows, (startc == xlBLACK) ? endc : startc, y_offset);
                            wxString ramp = wxString::Format("%lf", (double)(end_time - start_time) / 1000.0);
                            if (endc == xlBLACK) {
                                rd = ramp;
                            }
                            if (startc == xlBLACK) {
                                ru = ramp;
                            }
                        } else {
                            int time = wxAtoi(endms) - wxAtoi(startms);
                            int numFrames = time / SeqData.FrameTime();
                            xlColor color;
                            for (int x = 0; x < numFrames; x++) {
                                double ratio = x;
                                ratio /= numFrames;
                                color.Set(ChannelBlend(startc.Red(),endc.Red(),ratio),
                                          ChannelBlend(startc.Green(),endc.Green(),ratio),
                                          ChannelBlend(startc.Blue(),endc.Blue(),ratio));
                                wxString s = CreateSceneImage(imagePfx, wxString::Format("-%d", x+1),
                                                              element,
                                                              num_columns, num_rows, reverse_rows,
                                                              color, y_offset);
                                if (x == 0) {
                                    imageName = s;
                                }
                            }
                        }

                        wxString settings = _("E_CHECKBOX_Pictures_WrapX=0,E_CHOICE_Pictures_Direction=scaled,")
                            + "E_SLIDER_PicturesXC=0"
                            + ",E_SLIDER_PicturesYC=0"
                            + ",E_CHECKBOX_Pictures_PixelOffsets=1"
                            + ",E_TEXTCTRL_Pictures_Speed=1.0"
                            + ",E_TEXTCTRL_Pictures_FrameRateAdj=1.0"
                            + ",E_TEXTCTRL_Pictures_Filename=" + imageName
                            + ",T_CHECKBOX_LayerMorph=0,T_CHECKBOX_OverlayBkg=0,"
                            + "T_CHOICE_LayerMethod=Normal,T_SLIDER_EffectLayerMix=0,T_TEXTCTRL_Fadein=" + ru
                            + ",T_TEXTCTRL_Fadeout=" + rd;

                        layer->AddEffect(0, "Pictures", settings, "", start_time, end_time, false, false);
                    }
                }
            }
        } else if ("textActions" == e->GetName()) {
            for(wxXmlNode* element=e->GetChildren(); element!=NULL; element=element->GetNext()) {
                if ("textAction" == element->GetName()) {
                    wxString startms = element->GetAttribute("startCentisecond");
                    AppendConvertStatus("Could not map textAction at starting time " + startms + "0 ms\n", true);
                }
            }
        } else if ("imageActions" == e->GetName()) {
            for(wxXmlNode* element=e->GetChildren(); element!=NULL; element=element->GetNext()) {
                if ("imageAction" == element->GetName()) {
                    //<imageAction name="Image Action 14" colorType="nativeColor" maskType="normal" rotation="0" direction="8"
                    //  stopAtEdge="0" layer="3" xStart="-1" yStart="0" xEnd="0" yEnd="0" startCentisecond="115" endCentisecond="145"
                    //  preRampTime="0" rampTime="0" fadeToBright="0" fadeFromBright="0" imageIndex="5" savedIndex="0">

                    int idx = wxAtoi(element->GetAttribute("imageIndex"));
                    int startms = wxAtoi(element->GetAttribute("startCentisecond")) * 10;
                    int endms = wxAtoi(element->GetAttribute("endCentisecond")) * 10;
                    int layer_index = wxAtoi(element->GetAttribute("layer"));
                    int rampDownTime = wxAtoi(element->GetAttribute("rampTime")) * 10;
                    int rampUpTime = wxAtoi(element->GetAttribute("preRampTime")) * 10;
                    while( model->GetEffectLayerCount() <= layer_index ) {
                        model->AddEffectLayer();
                    }
                    wxString rampUpTimeString = "0";
                    if (rampUpTime) {
                        double fadeIn = rampUpTime;
                        fadeIn /= 1000;  //FadeIn is in seconds
                        rampUpTimeString = wxString::Format("%lf", fadeIn);
                    }
                    wxString rampDownTimeString = "0";
                    if (rampDownTime) {
                        double fade = rampDownTime;
                        fade /= 1000;  //FadeIn is in seconds
                        rampDownTimeString = wxString::Format("%lf", fade);
                    }


                    int startx = wxAtoi(element->GetAttribute("xStart"));
                    int starty = wxAtoi(element->GetAttribute("yStart"));
                    int endx = wxAtoi(element->GetAttribute("xEnd"));
                    int endy = wxAtoi(element->GetAttribute("yEnd"));


                    int xOffIfCentered =(imageInfo[idx].width-num_columns)/2;
                    int x = imageInfo[idx].xoffset + xOffIfCentered;



                    int yll = num_rows -  imageInfo[idx].yoffset;
                    int yOffIfCentered =(num_rows+imageInfo[idx].height)/2; //centered if sizes don't match
                    int y = yll - yOffIfCentered;


                    //yoffset+yoffset_adj-y - 1
                    /*
                    int xoffset =(imageInfo[idx].width-12)/2; //centered if sizes don't match
                    if (imageInfo[idx].xoffset != (0-xoffset + x)) {
                        printf("%d:  %d  %d  %d\n", idx, imageInfo[idx].width, imageInfo[idx].xoffset, startx);
                        printf("x    %d       %d\n", imageInfo[idx].xoffset + startx, x);
                        printf("     %d  \n", 0-xoffset + (x + startx));
                    }
                    int yoffset =(50+imageInfo[idx].height)/2; //centered if sizes don't match
                    if ((num_rows-(imageInfo[idx].yoffset + starty)) != (yoffset + (y - starty) - 1)) {
                        printf("%d:  %d  %d  %d\n", idx, imageInfo[idx].height, imageInfo[idx].yoffset, starty);
                        printf("y    %d       %d\n", int(num_rows) - (imageInfo[idx].yoffset + starty), y);
                        printf("     %d  \n", (yoffset + (y - starty) - 1));
                    }
                     */


                    layer = FindOpenLayer(model, layer_index, startms, endms, reserved);
                    if (endy == starty && endx == startx) {
                        x += startx;
                        y -= starty;
                        wxString settings = _("E_CHECKBOX_Pictures_WrapX=0,E_CHOICE_Pictures_Direction=none,")
                            + "E_SLIDER_PicturesXC=" + wxString::Format("%d", x)
                            + ",E_SLIDER_PicturesYC=" + wxString::Format("%d", y)
                            + ",E_CHECKBOX_Pictures_PixelOffsets=1"
                            + ",E_TEXTCTRL_Pictures_Filename=" + imageInfo[idx].imageName
                            + ",E_TEXTCTRL_Pictures_Speed=1.0"
                            + ",E_TEXTCTRL_Pictures_FrameRateAdj=1.0"
                            + ",T_CHECKBOX_LayerMorph=0,T_CHECKBOX_OverlayBkg=0,"
                            + "T_CHOICE_LayerMethod=Normal,T_SLIDER_EffectLayerMix=0,T_TEXTCTRL_Fadein=" + rampUpTimeString
                            + ",T_TEXTCTRL_Fadeout=" + rampDownTimeString;

                        layer->AddEffect(0, "Pictures", settings, "", startms, endms, false, false);
                    } else {
                        wxString settings = _("E_CHECKBOX_Pictures_WrapX=0,E_CHOICE_Pictures_Direction=vector,")
                            + "E_SLIDER_PicturesXC=" + wxString::Format("%d", x + startx)
                            + ",E_SLIDER_PicturesYC=" + wxString::Format("%d", y - starty)
                            + ",E_SLIDER_PicturesEndXC=" + wxString::Format("%d", x + endx)
                            + ",E_SLIDER_PicturesEndYC=" + wxString::Format("%d", y - endy)
                            + ",E_TEXTCTRL_Pictures_Speed=1.0"
                            + ",E_TEXTCTRL_Pictures_FrameRateAdj=1.0"
                            + ",E_CHECKBOX_Pictures_PixelOffsets=1"
                            + ",E_TEXTCTRL_Pictures_Filename=" + imageInfo[idx].imageName
                            + ",T_CHECKBOX_LayerMorph=0,T_CHECKBOX_OverlayBkg=0,"
                            + "T_CHOICE_LayerMethod=Normal,T_SLIDER_EffectLayerMix=0,T_TEXTCTRL_Fadein=" + rampUpTimeString
                            + ",T_TEXTCTRL_Fadeout=" + rampDownTimeString;

                        layer->AddEffect(0, "Pictures", settings, "", startms, endms, false, false);
                    }
                }
            }
        }
    }
    return true;
}

void AddLSPEffect(EffectLayer *layer, int pos, int epos, int in, int out, int eff, const wxColor &c, int bst, int ben) {
    if (eff == 4) {
        //off
        return;
    }
    xlColor color(c);
    xlColor color2(xlBLACK);

    bool isShimmer = eff == 5 || eff == 6;
    wxString effect = "On";
    wxString settings = wxString::Format("E_TEXTCTRL_Eff_On_End=%d,E_TEXTCTRL_Eff_On_Start=%d", out, in)
        + ",E_TEXTCTRL_On_Cycles=1.0,T_CHECKBOX_LayerMorph=0,T_CHECKBOX_OverlayBkg=0,"
        + "T_CHOICE_LayerMethod=Normal,T_SLIDER_EffectLayerMix=0,"
        + "T_TEXTCTRL_Fadein=0.00,T_TEXTCTRL_Fadeout=0.00,E_CHECKBOX_On_Shimmer=" + (isShimmer ? "1" : "0");


    if (bst != 0 && ben != 0) {
        color = xlColor(bst & 0xFFFFFF, false);
        color2 = xlColor(ben & 0xFFFFFF, false);
        if (color == color2) {
            color2 = xlBLACK;
        } else {
            effect = "Color Wash";
            settings = _("E_CHECKBOX_ColorWash_HFade=0,E_CHECKBOX_ColorWash_VFade=0,E_TEXTCTRL_ColorWash_Cycles=1.00,")
                + "E_TEXTCTRL_ColorWash_Cycles=1.00,E_CHECKBOX_ColorWash_CircularPalette=0,"
                + "T_CHECKBOX_LayerMorph=0,T_CHECKBOX_OverlayBkg=0,T_CHOICE_LayerMethod=Normal,"
                + "T_SLIDER_EffectLayerMix=0,T_TEXTCTRL_Fadein=0.00,T_TEXTCTRL_Fadeout=0.00"
                + ",E_CHECKBOX_ColorWash_Shimmer=" + (isShimmer ? "1" : "0");
        }
    }

    if (xlBLACK == color && xlBLACK == color2) {
        //nutcracker or other effects imported into LSP generate "BLACK" effects in the sequence.  Don't import them.
        return;
    }

    wxString palette = _("C_BUTTON_Palette1=" + color + ",C_CHECKBOX_Palette1=1,C_BUTTON_Palette2=") + color2
        + ",C_CHECKBOX_Palette2=1,C_CHECKBOX_Palette3=0,C_CHECKBOX_Palette4=0,C_CHECKBOX_Palette5=0,C_CHECKBOX_Palette6=0,"
        + "C_SLIDER_Brightness=100,C_SLIDER_Contrast=0,C_SLIDER_SparkleFrequency=0";

    int start_time = (int)(pos * 50.0 / 4410.0);
    int end_time = (int)((epos - 1) * 50.0 / 4410.0);
    layer->AddEffect(0, effect, settings, palette, start_time, end_time, false, false);
}

void MapLSPEffects(EffectLayer *layer, wxXmlNode *node, const wxColor &c) {
    if (node == nullptr) {
        return;
    }
    int eff = -1;
    int in, out, pos;

    int bst, ben;


    for (wxXmlNode *cnd = node->GetChildren(); cnd != nullptr; cnd = cnd->GetNext()) {
        if (cnd->GetName() == "Tracks") {
            for (wxXmlNode *cnnd = cnd->GetChildren(); cnnd != nullptr; cnnd = cnnd->GetNext()) {
                if (cnnd->GetName() == "Track") {
                    for (wxXmlNode *ind = cnnd->GetChildren(); ind != nullptr; ind = ind->GetNext()) {
                        if (ind->GetName() == "Intervals") {
                            for (wxXmlNode *ti = ind->GetChildren(); ti != nullptr; ti = ti->GetNext()) {
                                if (ti->GetName() == "TimeInterval") {
                                    int neff = wxAtoi(ti->GetAttribute("eff", "4"));
                                    if (eff != -1 && neff != 7) {
                                        int npos = wxAtoi(ti->GetAttribute("pos", "1"));
                                        AddLSPEffect(layer, pos, npos, in, out, eff, c, bst, ben);
                                    }
                                    if (neff != 7) {
                                        pos = wxAtoi(ti->GetAttribute("pos", "1"));
                                        eff = neff;
                                        in = wxAtoi(ti->GetAttribute("in", "1"));
                                        out = wxAtoi(ti->GetAttribute("out", "1"));
                                        bst = wxAtoi(ti->GetAttribute("bst", "0"));
                                        ben = wxAtoi(ti->GetAttribute("ben", "0"));
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void MapLSPStrand(StrandLayer *layer, wxXmlNode *node, const wxColor &c) {
    int nodeNum = 0;
    for (wxXmlNode *nd = node->GetChildren(); nd != nullptr; nd = nd->GetNext()) {
        if (nd->GetName() == "Channels") {
            for (wxXmlNode *cnd = nd->GetChildren(); cnd != nullptr; cnd = cnd->GetNext()) {
                if (cnd->GetName() == "Channel") {
                    EffectLayer *el = layer->GetNodeLayer(nodeNum);
                    MapLSPEffects(el, cnd, c);
                    nodeNum++;
                    if (nodeNum >= layer->GetNodeLayerCount()) {
                        return;
                    }
                }
            }
        }
    }
}

void xLightsFrame::ImportLSP(const wxFileName &filename) {
    wxStopWatch sw; // start a stopwatch timer

    LMSImportChannelMapDialog dlg(this);
    dlg.mSequenceElements = &mSequenceElements;
    dlg.xlights = this;


    wxFileName msq_file(filename);
    wxString msq_doc = msq_file.GetFullPath();
    wxFileInputStream fin(msq_doc);
    wxZipInputStream zin(fin);
    wxZipEntry *ent = zin.GetNextEntry();


    wxXmlDocument seq_xml;
    std::map<wxString, wxXmlDocument> cont_xml;
    std::map<wxString, wxXmlNode *> nodes;
    std::map<wxString, wxXmlNode *> strandNodes;

    while (ent != nullptr) {
        if (ent->GetName() == "Sequence") {
            seq_xml.Load(zin);
        } else {
            wxString id("1");
            wxXmlDocument &doc =  cont_xml[ent->GetName()];

            if (doc.Load(zin)) {
                for (wxXmlNode *nd = doc.GetRoot()->GetChildren(); nd != nullptr; nd = nd->GetNext()) {
                    if (nd->GetName() == "ControllerName") {
                        id = nd->GetChildren()->GetContent();
                    }
                }
                strandNodes[id] = doc.GetRoot();
                dlg.ccrNames.push_back(id);
                for (wxXmlNode *nd = doc.GetRoot()->GetChildren(); nd != nullptr; nd = nd->GetNext()) {
                    if (nd->GetName() == "Channels") {
                        for (wxXmlNode *cnd = nd->GetChildren(); cnd != nullptr; cnd = cnd->GetNext()) {
                            if (cnd->GetName() == "Channel") {
                                wxString cname;
                                for (wxXmlNode *cnnd = cnd->GetChildren(); cnnd != nullptr; cnnd = cnnd->GetNext()) {
                                    if (cnnd->GetName() == "Tracks") {
                                        for (wxXmlNode *tnd = cnnd->GetChildren(); tnd != nullptr; tnd = tnd->GetNext()) {
                                            if (tnd->GetName() == "Track") {
                                                for (wxXmlNode *tnd2 = tnd->GetChildren(); tnd2 != nullptr; tnd2 = tnd2->GetNext()) {
                                                    if (tnd2->GetName() == "Name") {
                                                        cname = tnd2->GetChildren()->GetContent();
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                                cname = id + "/" + cname;
                                nodes[cname] = cnd;
                                dlg.channelNames.push_back(cname);
                                dlg.channelColors[cname] = xlWHITE;
                            }
                        }
                    }
                }
            } else {
                wxLogError("Could not parse XML file %s", (const char *)ent->GetName());
            }
        }
        ent = zin.GetNextEntry();
    }

    dlg.channelNames.Sort();
    dlg.channelNames.Insert("", 0);
    dlg.ccrNames.Sort();
    dlg.ccrNames.Insert("", 0);

    dlg.Init();

    if (dlg.ShowModal() != wxID_OK) {
        return;
    }

    int row = 0;
    for (int m = 0; m < dlg.modelNames.size(); m++) {
        wxString modelName = dlg.modelNames[m];
        ModelClass *mc = GetModelClass(modelName);
        Element * model = nullptr;
        for (int i=0;i<mSequenceElements.GetElementCount();i++) {
            if (mSequenceElements.GetElement(i)->GetType() == "model"
                && modelName == mSequenceElements.GetElement(i)->GetName()) {
                model = mSequenceElements.GetElement(i);
            }
        }
        if (dlg.ChannelMapGrid->GetCellValue(row, 3) != "" && !dlg.MapByStrand->IsChecked()) {
            MapLSPEffects(model->GetEffectLayer(0), nodes[dlg.ChannelMapGrid->GetCellValue(row, 3)],
                          dlg.ChannelMapGrid->GetCellBackgroundColour(row, 4));
        }
        row++;

        for (int str = 0; str < mc->GetNumStrands(); str++) {
            StrandLayer *sl = model->GetStrandLayer(str);

            if ("" != dlg.ChannelMapGrid->GetCellValue(row, 3)) {
                if (dlg.MapByStrand->IsChecked()) {
                    MapLSPStrand(sl, strandNodes[dlg.ChannelMapGrid->GetCellValue(row, 3)],
                                  dlg.ChannelMapGrid->GetCellBackgroundColour(row, 4));
                } else {
                    MapLSPEffects(sl, nodes[dlg.ChannelMapGrid->GetCellValue(row, 3)],
                                  dlg.ChannelMapGrid->GetCellBackgroundColour(row, 4));
                }
            }
            row++;
            if (!dlg.MapByStrand->IsChecked()) {
                for (int n = 0; n < mc->GetStrandLength(str); n++) {
                    if ("" != dlg.ChannelMapGrid->GetCellValue(row, 3)) {
                        NodeLayer *nl = sl->GetNodeLayer(n);
                        MapLSPEffects(nl, nodes[dlg.ChannelMapGrid->GetCellValue(row, 3)],
                                      dlg.ChannelMapGrid->GetCellBackgroundColour(row, 4));
                    }
                    row++;
                }
            }
        }
    }

    float elapsedTime = sw.Time()/1000.0; //msec => sec
    StatusBar1->SetStatusText(wxString::Format("'%s' imported in %4.3f sec.", filename.GetPath(), elapsedTime));
}

