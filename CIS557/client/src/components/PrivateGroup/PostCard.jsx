import React from 'react';
import { useNavigate } from 'react-router-dom';
import Card from '@mui/material/Card';
import CardHeader from '@mui/material/CardHeader';
import CardMedia from '@mui/material/CardMedia';
import CardContent from '@mui/material/CardContent';
import CardActionArea from '@mui/material/CardActionArea';
import CardActions from '@mui/material/CardActions';
import { Button, Typography, IconButton } from '@mui/material';

import Forum from '@mui/icons-material/Forum';
import Share from '@mui/icons-material/Share';
import ReportProblem from '@mui/icons-material/ReportProblem';
import Delete from '@mui/icons-material/Delete';
import VisibilityOff from '@mui/icons-material/VisibilityOff';

import * as fetchPost from '../../utils/fetchPrivatePost';
import * as fetchGroup from '../../utils/fetchPrivateGroup';
import * as fetchUser from '../../utils/fetchUser';

import * as utils from '../../utils/utils';
import { baseURL } from '../../utils/utils';

export default function PostCard({ post, userInfo, group }) {
  const navigate = useNavigate();
  const cardStyle = {
    display: 'block',
    width: '800px',
    marginBottom: '25px',
  };

  const handleClick = (post) => {
    navigate(`/privatepost/${post._id}`);
  };

  const handleClickFlagForDel = async () => {
    await fetchGroup.flagForDeletion(post.group, userInfo.id, post._id);
    await fetchPost.flagForDeletion(post._id, userInfo.id);
    navigate(0);
  };

  const handleClickHide = async () => {
    await fetchUser.hidePost(userInfo.id, post._id, post.group, 'private');
    navigate(0);
  };

  const handleClickDelete = async () => {
    await fetchGroup.deletePost(post._id, post.group);
    navigate(0);
  };

  const canDelete = userInfo ? (group.admins.indexOf(userInfo.id) >= 0) : false;
  const canHide = userInfo ? (post.author !== userInfo.id) && utils.checkUserInPrivateGroup(userInfo, group.id) : false;

  return (
    <div>
      <Card style={cardStyle} elevation={3}>

        <CardHeader
          action={(
            <div>
              {canHide ? <IconButton onClick={handleClickHide}><VisibilityOff /></IconButton> : null}
              {canDelete ? <IconButton onClick={handleClickDelete}><Delete /></IconButton> : null}
            </div>
)}
          title={post.title}
          subheader={`${utils.convertTime(post.date)} Written by ${post.author} from ${post.group}`}
        />
        <CardActionArea onClick={() => handleClick(post)}>
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
        </CardActionArea>
        <CardActions>
          <Button size="small" sx={{ color: '#000000', '&:hover': { background: '#f5f5f5' } }} onClick={() => handleClick(post)} startIcon={<Forum />}>
            {post.comments.length}
            {' '}
            Comments
          </Button>

          <Button size="small" sx={{ color: '#000000', '&:hover': { background: '#f5f5f5' } }} startIcon={<Share />}>
            Share
          </Button>
          {utils.checkUserInPrivateGroup(userInfo, post.group)
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
    </div>
  );
}
