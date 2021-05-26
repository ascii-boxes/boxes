---
title: Editor Integration - JED
permalink: editors-jed.html
created_at: 1999-04-06
last_modified_at: 2014-12-23 22:50:00 +0100
---

<a href="{{ site.baseurl }}/editors.html"><button type="button" class="btn btn-outline-primary">Back to All Editors</button></a>


# Integration with Jed

<span class="atmention">[@asheiduk](https://github.com/asheiduk)</span> shared the following excerpt from his *.jedrc*:

    %!% Ripped from "pipe.sl"
    
    variable Last_Process_Command = Null_String;
    
    define do_process_region(cmd) {
       variable tmp;
       tmp = make_tmp_file ("/tmp/jedpipe");
       cmd = strncat (cmd, " > ", tmp, " 2>&1", 4);
    
       !if (dupmark ()) error ("Mark not set.");
    
       if (pipe_region (cmd))
       {
          error ("Process returned a non-zero exit status.");
       }
       del_region ();
       () = insert_file (tmp);
       () = delete_file (tmp);
    }
    
    
    define process_region ()
    {
       variable cmd;
       cmd = read_mini ("Pipe to command:", Last_Process_Command, "");
       !if (strlen (cmd)) return;
    
       Last_Process_Command = cmd;
       do_process_region(cmd);
    }
    
    
    %-----------------------------------------------------------------------
    
    if( BATCH == 0 ){
    
       setkey("process_region",	"\e|");		% ESC-Pipe :-)
       add_completion("process_region");
       
       % define some often used filters
       setkey("do_process_region(\"tal\")",	"\et")	% tal on esc-t
    }

I think it calls [tal](https://thomasjensen.com/software/tal/) when you press `ESC-t` (second but last line). Thus,
you would have to add a similar line to call *boxes*.
