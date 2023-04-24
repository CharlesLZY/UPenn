import React, { useState, useRef } from 'react';
import { useNavigate } from 'react-router-dom';
import { makeStyles } from '@mui/styles';
import {
  Paper, IconButton, Button, Typography, TextField, Divider,
} from '@mui/material';
import Menu from '@mui/material/Menu';
import MenuItem from '@mui/material/MenuItem';

import MoreVert from '@mui/icons-material/MoreVert';
import Close from '@mui/icons-material/Close';
import ArrowUpward from '@mui/icons-material/ArrowUpward';

import * as fetchPost from '../../utils/fetchPrivatePost';
import * as utils from '../../utils/utils';

const useStyles = makeStyles({
  root: {
    margin: 10,
    display: 'flex',
    justifyContent: 'space-between',

  },
  text: {
    width: '750px',
    wordWrap: 'break-word',
  },
  content: {
    marginLeft: 10,
  },
  action: {
  },
  input: {
    marginLeft: 20,
    display: 'flex',
  },
});

export default function CommentCard({ comment, userID, postID }) {
  const classes = useStyles();
  const navigate = useNavigate();

  const [anchor, setAnchor] = useState(null);
  const [edit, setEdit] = useState(false);
  const [editContent, seteditContent] = useState('');

  const textInput = useRef(null);

  const handleClick = (e) => {
    setAnchor(e.currentTarget);
  };

  const handleClose = () => {
    setAnchor(null);
  };

  const handleClickEdit = () => {
    setEdit(true);
  };

  const handleSubmitEdit = () => {
    if (editContent !== '') {
      console.log(editContent);
      textInput.current.value = '';
    }
  };

  const handleClickDelete = async (comment) => {
    await fetchPost.deleteComment(postID, comment.author, comment.date);
    navigate(0);
  };

  return (
    <Paper square elevation={0}>
      <div className={classes.root}>
        <div>
          <Divider sx={{ marginLeft: 1, marginBottom: 1, width: '770px' }} />
          <div className={classes.content}>
            <Typography variant="caption">
              {`${utils.convertTime(comment.date)} Written by ${comment.author}`}
            </Typography>
          </div>
          <div className={classes.content}>
            <Typography variant="body1" className={classes.text}>
              {comment.text}
            </Typography>
          </div>

        </div>
        <div className={classes.action}>
          {userID === comment.author
            ? (
              <IconButton size="small" color="default" onClick={handleClick}>
                <MoreVert />
              </IconButton>
            )
            : <div />}

          <Menu
            id="authorMenu"
            anchorEl={anchor}
            open={Boolean(anchor)}
            onClose={handleClose}
            anchorOrigin={{
              vertical: 'top',
              horizontal: 'center',
            }}
            transformOrigin={{
              vertical: 'top',
              horizontal: 'center',
            }}
          >
            <MenuItem onClick={handleClickEdit}>Edit</MenuItem>
            <MenuItem onClick={() => handleClickDelete(comment)}>Delete</MenuItem>
          </Menu>

        </div>
      </div>
      {edit
                && (
                <div className={classes.input}>
                  <TextField
                    variant="outlined"
                    color="secondary"
                    label="Edit your comment."
                    multiline
                    maxRows={2}
                    fullWidth
                    defaultValue={comment.text}
                    type="text"
                    inputRef={textInput}
                    inputProps={{ maxLength: 265 }}
                    onChange={(e) => seteditContent(e.target.value)}
                  />
                  <Button onClick={handleSubmitEdit} sx={{ color: '#000000', '&:hover': { background: '#f5f5f5' } }}>
                    <ArrowUpward />
                  </Button>
                  <Button onClick={() => setEdit(false)} sx={{ color: '#000000', '&:hover': { background: '#f5f5f5' } }}>
                    <Close />
                  </Button>
                </div>
                )}
    </Paper>
  );
}
