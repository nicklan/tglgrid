proc _ {str} {
    return $str
}

proc configure_canvas {id canv} {
    set vid [string trimleft $id .]
    set var_tglgui_cell_size [concat tglgui_cell_size_$vid]
    global $var_tglgui_cell_size
    set var_tglgui_spacing [concat tglgui_spacing_$vid]
    global $var_tglgui_spacing
    set var_tglgui_tglfill [concat tglgui_tglfill_$vid]
    global $var_tglgui_tglfill
    set var_tglgui_untglfill [concat tglgui_untglfill_$vid]
    global $var_tglgui_untglfill


    set cellsize  [eval concat $$var_tglgui_cell_size]
    set spacing   [eval concat $$var_tglgui_spacing]
    set tglfill   [eval concat $$var_tglgui_tglfill]
    set untglfill [eval concat $$var_tglgui_untglfill]
    set cdim [expr {($cellsize * 2) + ($spacing * 3)}]


    $canv configure -width $cdim -height $cdim

    $canv coords $id.previewcell0 \
        $spacing $spacing [expr $cellsize+$spacing] [expr $cellsize+$spacing]
    $canv coords $id.previewcell1 \
        [expr $spacing*2 + $cellsize] $spacing [expr $spacing*2 + $cellsize*2] [expr $cellsize+$spacing]
    $canv coords $id.previewcell2 \
        $spacing [expr $spacing*2 + $cellsize] [expr $cellsize+$spacing] [expr $spacing*2 + $cellsize*2]
    $canv coords $id.previewcell3 \
        [expr $spacing*2 + $cellsize] [expr $spacing*2 + $cellsize] \
        [expr $spacing*2 + $cellsize*2] [expr $spacing*2 + $cellsize*2]

    $canv itemconfigure $id.previewcell0 -fill $tglfill
    $canv itemconfigure $id.previewcell1 -fill $untglfill
    $canv itemconfigure $id.previewcell2 -fill $untglfill
    $canv itemconfigure $id.previewcell3 -fill $tglfill
}

proc layout_changed {id canv newval spc} {
    if {$newval == ""} {
        return 1
    }
    if [string is integer $newval] {
        set vid [string trimleft $id .]
        set changed 0
        if {$spc == 0} {
            set var_tglgui_cell_size [concat tglgui_cell_size_$vid]
            global $var_tglgui_cell_size
            if {[eval concat $$var_tglgui_cell_size] != $newval} {
                set $var_tglgui_cell_size $newval
                set changed 1
            }
        } else {
            set var_tglgui_spacing [concat tglgui_spacing_$vid]
            global $var_tglgui_spacing
            if {[eval concat $$var_tglgui_spacing] != $newval} {
                set $var_tglgui_spacing $newval
                set changed 1
            }
        }
        if $changed {
            configure_canvas $id $canv
        }
        return 1
    } else {
        return 0
    }
}

proc handle_color {id canv tgl} {
    set vid [string trimleft $id .]
    if {$tgl == 0} {
        set var_tglgui_tglfill [concat tglgui_tglfill_$vid]
        global $var_tglgui_tglfill
        set selected_color [tk_chooseColor -initialcolor [eval concat $$var_tglgui_tglfill]]
        set $var_tglgui_tglfill $selected_color
    } else {
        set var_tglgui_untglfill [concat tglgui_untglfill_$vid]
        global $var_tglgui_untglfill
        set selected_color [tk_chooseColor -initialcolor [eval concat $$var_tglgui_untglfill]]
        set $var_tglgui_untglfill $selected_color
    }
    configure_canvas $id $canv
}

proc tglgrid_apply {id} {
    set vid [string trimleft $id .]

    set var_tglgui_cols [concat tglgui_cols_$vid]
    global $var_tglgui_cols
    set var_tglgui_rows [concat tglgui_rows_$vid]
    global $var_tglgui_rows
    set var_tglgui_cell_size [concat tglgui_cell_size_$vid]
    global $var_tglgui_cell_size
    set var_tglgui_spacing [concat tglgui_spacing_$vid]
    global $var_tglgui_spacing
    set var_tglgui_tglfill [concat tglgui_tglfill_$vid]
    global $var_tglgui_tglfill
    set var_tglgui_untglfill [concat tglgui_untglfill_$vid]
    global $var_tglgui_untglfill

    pdsend [concat $id dialog \
                [eval concat $$var_tglgui_cols] \
                [eval concat $$var_tglgui_rows] \
                [eval concat $$var_tglgui_cell_size] \
                [eval concat $$var_tglgui_spacing] \
                [eval concat $$var_tglgui_tglfill] \
                [eval concat $$var_tglgui_untglfill]]
}

proc tglgrid_cancel {id} {
    pdsend "$id cancel"
}

proc tglgrid_ok {id} {
    tglgrid_apply $id
    tglgrid_cancel $id
}

proc tglgrid_dialog {id name cols rows cell_size spacing tglfill untglfill canvcol} {
    set vid [string trimleft $id .]

    set var_tglgui_cols [concat tglgui_cols_$vid]
    global $var_tglgui_cols
    set var_tglgui_rows [concat tglgui_rows_$vid]
    global $var_tglgui_rows

    set var_tglgui_cell_size [concat tglgui_cell_size_$vid]
    global $var_tglgui_cell_size
    set var_tglgui_spacing [concat tglgui_spacing_$vid]
    global $var_tglgui_spacing

    set var_tglgui_whichcolor [concat tglgui_whichcolor_$vid]
    global $var_tglgui_whichcolor
    set var_tglgui_tglfill [concat tglgui_tglfill_$vid]
    global $var_tglgui_tglfill
    set var_tglgui_untglfill [concat tglgui_untglfill_$vid]
    global $var_tglgui_untglfill

    set $var_tglgui_cols $cols
    set $var_tglgui_rows $rows
    set $var_tglgui_cell_size $cell_size
    set $var_tglgui_spacing $spacing
    set $var_tglgui_whichcolor 0

    set $var_tglgui_tglfill $tglfill
    set $var_tglgui_untglfill $untglfill

    toplevel $id -class DialogWindow
    wm title $id "tglgrid Properties"
    wm resizable $id 0 0
    wm transient $id $::focused_window
    #$id configure -menu $::dialog_menubar
    $id configure -padx 0 -pady 0

    labelframe $id.dim -borderwidth 1 -text [_ "Grid Size"]
    pack $id.dim -fill x -ipadx 5 -ipady 4 -pady 10 -padx 5
    label $id.dim.dummy1 -text " " -width 5
    label $id.dim.col_lab -text "Columns: " -width 10
    entry $id.dim.col_ent -textvariable $var_tglgui_cols -width 5
    label $id.dim.row_lab -text "Rows: " -width 10
    entry $id.dim.row_ent -textvariable $var_tglgui_rows -width 5
    pack $id.dim.col_lab $id.dim.col_ent $id.dim.dummy1 $id.dim.row_lab $id.dim.row_ent -side left

    labelframe $id.space -borderwidth 1 -text [_ "Cell Dimensions"]
    pack $id.space -fill x -ipadx 5 -ipady 4 -pady 10 -padx 5
    label $id.space.dummy1 -text " " -width 5
    label $id.space.size_lab -text [_ "Cell Size: "] -width 10
    entry $id.space.size_ent -width 5 \
        -validate all -validatecommand "layout_changed $id $id.preview.previewcell %P 0"
    $id.space.size_ent insert 0 [eval concat $$var_tglgui_cell_size]
    label $id.space.space_lab -text [_ "Spacing: "] -width 10
    entry $id.space.space_ent -width 5 \
        -validate all -validatecommand "layout_changed $id $id.preview.previewcell %P 1"
    $id.space.space_ent insert 0 [eval concat $$var_tglgui_spacing]
    puts [$id.space.space_ent configure]
    pack $id.space.size_lab $id.space.size_ent $id.space.dummy1 $id.space.space_lab $id.space.space_ent -side left

    #COLORS
    labelframe $id.colors -borderwidth 1 -text [_ "Colors"]
    pack $id.colors -fill x -ipadx 5 -ipady 4 -pady 10 -padx 5

    frame $id.colors.select
    pack $id.colors.select -side top
    button $id.colors.select.tglfill -text [_ "Toggled Color"] \
        -command "handle_color $id $id.preview.previewcell 0"
    button $id.colors.select.untglfill -text [_ "Un-Toggled Color"] \
        -command "handle_color $id $id.preview.previewcell 1"

    pack $id.colors.select.tglfill $id.colors.select.untglfill -side left

    labelframe $id.preview -borderwidth 1 -text [_ "Preview"]
    pack $id.preview -fill x -ipadx 5 -ipady 4 -pady 10 -padx 5
    canvas $id.preview.previewcell -background $canvcol
    $id.preview.previewcell create rectangle 0 0 0 0 -tags $id.previewcell0
    $id.preview.previewcell create rectangle 0 0 0 0 -tags $id.previewcell1
    $id.preview.previewcell create rectangle 0 0 0 0 -tags $id.previewcell2
    $id.preview.previewcell create rectangle 0 0 0 0 -tags $id.previewcell3
    configure_canvas $id $id.preview.previewcell
    pack $id.preview.previewcell


    frame $id.buttonframe
    pack $id.buttonframe -side bottom -fill x -pady 2m
    button $id.buttonframe.cancel -text {Cancel}\
        -command "tglgrid_cancel $id"
    button $id.buttonframe.apply -text {Apply}\
        -command "tglgrid_apply $id"
    button $id.buttonframe.ok -text {OK}\
        -command "tglgrid_ok $id"
    pack $id.buttonframe.cancel -side left -expand 1
    pack $id.buttonframe.apply -side left -expand 1
    pack $id.buttonframe.ok -side left -expand 1
}
