import React, { useState, useEffect, useRef } from 'react';
import { useNavigate, useParams } from 'react-router-dom';
import { makeStyles } from '@mui/styles';
import Card from '@mui/material/Card';
import CardHeader from '@mui/material/CardHeader';
import CardMedia from '@mui/material/CardMedia';
import CardContent from '@mui/material/CardContent';
import CardActions from '@mui/material/CardActions';
import {
  Container, Paper, Typography, Button, TextField, IconButton, Divider,
} from '@mui/material';

import Forum from '@mui/icons-material/Forum';
import Share from '@mui/icons-material/Share';
import Home from '@mui/icons-material/Home';
import ReportProblem from '@mui/icons-material/ReportProblem';
import ArrowUpward from '@mui/icons-material/ArrowUpward';
import Close from '@mui/icons-material/Close';
import FilterListIcon from '@mui/icons-material/FilterList';

import Layout from '../../components/Layout/Layout';
import Comment from '../../components/PublicGroup/Comment';

import * as fetchUser from '../../utils/fetchUser';
import * as fetchPost from '../../utils/fetchPublicPost';
import * as fetchGroup from '../../utils/fetchPublicGroup';
import * as utils from '../../utils/utils';
import { baseURL } from '../../utils/utils';

const useStyles = makeStyles({
  postSectionBG: {
    marginTop: 20,
    width: '850px',
    paddingBottom: 20,
  },

  input: {
    marginLeft: 20,
    marginBottom: 10,
    display: 'flex',
  },

  search: {
    marginTop: 10,
    marginLeft: 20,
  },
});

export default function PublicPost() {
  const classes = useStyles();
  const userID = sessionStorage.getItem('userID');
  const params = useParams();
  const { postID } = params;
  const navigate = useNavigate();

  const [post, setPost] = useState(null);
  const [userInfo, setUserInfo] = useState(null);
  const [tag, setTag] = useState('');
  const [commentsToshow, setCommentsToShow] = useState([]);

  const [input, setInput] = useState('');

  const textInput = useRef(null);

  const handleClickHome = (groupID) => {
    navigate(`/publicgroup/${groupID}`);
  };

  const handleReset = () => {
    document.getElementById('hashtag-inputBar').value = '';
    setTag('');
    setCommentsToShow(post.comments);
  };

  const handleFilter = () => {
    setCommentsToShow(post.comments.filter((comment) => comment.hashtags.indexOf(tag) >= 0));
  };

  const handleClickFlagForDel = async () => {
    await fetchGroup.flagForDeletion(post.group, userID, post._id);
    await fetchPost.flagForDeletion(post._id, userID);
    navigate(0);
  };

  const handleSubmit = async () => {
    if (input !== '') {
      textInput.current.value = '';
      await fetchPost.commentPost(postID, userID, input, post.group);
      navigate(0);
    }
  };

  useEffect(async () => {
    const data = await fetchPost.fetchPost(postID);
    if (data === null) {
      navigate('/wrongpage');
    } else {
      data.comments.sort((c1, c2) => {
        if (c1.date > c2.date) return -1;
        if (c1.date < c2.date) return 1;
        return 0;
      });
      setPost(data);
      setCommentsToShow(data.comments);
      if (userID) {
        const user = await fetchUser.fetchUser(userID);
        setUserInfo(user);
      }
    }
  }, []);

  function renderComments() {
    if (post) {
      return (
        commentsToshow.map((comment) => (
          <Comment comment={comment} userID={userID} postID={postID} key={`${comment.author}${comment.date}`} />
        ))
      );
    }
  }

  return (
    <Layout>
      <Container sx={{ display: 'inlined', justifyContent: 'center', paddingBottom: 5 }}>
        <Paper className={classes.postSectionBG} square elevation={2}>
          { post
            ? (
              <Card elevation={0}>
                <CardHeader
                  action={<IconButton onClick={() => handleClickHome(post.group)}><Home /></IconButton>}
                  title={post.title}
                  subheader={`${utils.convertTime(post.date)} Written by ${post.author} from ${post.group}`}
                />
                <CardContent>
                  <Typography variant="h6">
                    {post.content}
                  </Typography>
                </CardContent>
                <CardMedia
                  component={utils.parseFileType(post.attachment.fileType)}
                  src={post.attachment.fileID ? `${baseURL}/file/${post.attachment.fileID}` : null}
                  controls
                />
                <CardActions>
                  <Button size="small" sx={{ color: '#000000', '&:hover': { background: '#f5f5f5' } }} startIcon={<Forum />}>
                    {post.comments.length}
                    {' '}
                    Comments
                  </Button>

                  <Button size="small" sx={{ color: '#000000', '&:hover': { background: '#f5f5f5' } }} startIcon={<Share />}>
                    Share
                  </Button>
                  {utils.checkUserInPublicGroup(userInfo, post.group)
                    ? (
                      <Button
                        size="small"
                        sx={{ color: '#000000', '&:hover': { background: '#f5f5f5' } }}
                        startIcon={<ReportProblem />}
                        onClick={handleClickFlagForDel}
                        disabled={post.flags.indexOf(userInfo.id) >= 0}
                      >
                        Request for deletion
                      </Button>
                    )
                    : <div />}

                </CardActions>
              </Card>
            )
            : <div />}
          {post && utils.checkUserInPublicGroup(userInfo, post.group)
            ? (
              <div className={classes.input}>
                <TextField
                  id="inputBar2"
                  variant="outlined"
                  color="secondary"
                  label="Write your comment."
                  multiline
                  rows={2}
                  fullWidth
                  type="text"
                  inputRef={textInput}
                  inputProps={{ maxLength: 265 }}
                  onChange={(e) => setInput(e.target.value)}
                />
                <Button onClick={handleSubmit} sx={{ color: '#000000', '&:hover': { background: '#f5f5f5' } }}>
                  <ArrowUpward />
                </Button>
              </div>
            )
            : <div />}
          <Divider />
          <div className={classes.search}>
            <TextField
              size="small"
              color="secondary"
              label="Hashtag"
              id="hashtag-inputBar"
              inputProps={{ maxLength: 30 }}
              onChange={(e) => setTag(e.target.value)}
            />
            <IconButton onClick={handleReset}>
              <Close />
            </IconButton>
            <IconButton onClick={handleFilter}>
              <FilterListIcon />
            </IconButton>
          </div>
          {renderComments()}
        </Paper>

      </Container>
    </Layout>
  );
}
